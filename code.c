/**
 * Comotti Daniel 10706085 Progetto di Algoritmi e Strutture Dati AA 2022-2023
* This file is a script able to identify the most efficient journey (less stops) from a given start point A to a given endpoint B.
* those points are stations placed in a straight line highway. Each station has at max 512 cars, each one with its autonomy.
* if we decide to stop into a station we are forced to pick a new car from current station to continue journey.
*
* How the program works:
 * constantly reading from StandardInput (until EOF) and printing a response to StandardOutput.
 * types of input:
 * - aggiungi-stazione distanza numero-auto autonomia-auto-1 ... autonomia-auto-n :es: aggiungi-stazione 10 3 100 200 300 :output: aggiunta | non aggiunta
 * - demolisci-stazione distanza :es: demolisci-stazione 10 :output: demolita | non demolita
 * - aggiungi-auto distanza-stazione autonomia-auto-da-aggiungere :es: aggiungi-auto 10 54312 :output: aggiunta | non aggiunta
 * - rottama-auto distanza-stazione autonomia-auto-da-rottamare :es: rottama-auto 10 300 :output: rottamata | non rottamata
 * - pianifica-percorso distanza-stazione-partenza distanza-stazione-arrivo :es: pianifica-percorso 50 10 :output: 50 30 15 10 | nessun percorso
*
* How the program REALLY works:
 * we read 1 character per time (using getc). gestisciFlussoDiEsecuzione() collects all characters before \n and understands which kind of operation we need to perform. It can call:
 *  - aggiungiStazione(kilometer of the station to be added)
 *  - aggiungiAuto(kilomter of the station where to add the car, autonomy of the car to add, printFlag): printFlag = true --> aggiungiAuto called from a
 *    aggiungi-auto command; printFlag = false ----> aggiungiAuto called from aggiungi-stazione command.
 *  - rimuoviAuto(kilomter of the station where to remove the car, autonomy of the car to be removed)
 *  - rimuoviStazione(kilometer of the station to be removed)
 *  - pianificaPercorso(kilometer of start station, kilometer of end station)
 *
* How data structure is made:
 * Each station is a struct that memorizes the kilometer, its max auto, all cars autonomy and the number of cars available. it also has 4 pointers:
 * - maxNextStazione is the maximun next station that can reach ME
 * - maxPrevStazione is the minimum previus station that can reach ME
 * - nextStazone and preStazione are pointers to neighbour stations
 * - puntDS points to where this station is accessible from custom dataStructure. Its incredibly useful during eliminations.
 *   this pointer happened to be also useful for debugging: because using a void pointer my IDE's debugger wasn't able to resolve it. But puntDS
 *   allows me to travel DS in reverse order (from children to father).
 *
 * Each station is quickly accessible using my HashTable. It can be easily resized modifying dimHashTable.
 * it works exactly as a standard hash table: each kilometer is converted to a number lower than 100.000 and this new number is my entry.
 * of course collison can happen: we create a linked list (the item are called buckets) to fit more than one pointer to its station into one table entry.
 * this is very fast if we need a station or verify if a station is present or not.
 *
 * Each station (of course I mean pointer to the struct station) is also memorized into my "custom tree". I dont know if there's
 * a specific name for this tree but, since I haven't seen it during lessons and it basically came out of my head during one of the
 * craziest night of all times, we will simply call it daniel'sTree :) .
 * How daniel'sTree Works:
 *  - the basic node is a struct vettoreCifre: has 1 pointer to its father (NULL if root), one char used to count
 *    (since I only need to count numbers from 0 to 10 one int will occupy unnecessary space. 8 bit are enough for me)
 *    and an array of 10 struct Cifra. This struct Cifra has 1 validity bit and 1 void pointer. This pointer will always point
 *    to NULL or another vettoreCifre. Only if we are at leafs this void pinter will point to a real station. And puntDS of
 *    the station will point to the vettoreCifre contaning the cifra pointing to the station.
 * - the root is a vettoreCifre called inizio: here each entry of the array contained represents the start point for a tree
 *    containing only station which kilometer's number of characters is the index of the array. Of course inizio->c[0].x will always
 *    be false (the minimum station would be 0 and it has 1 character). cont represents the number of entry of the array (min 0 max 10)
 *    that actually have something attached. Its useful for an efficient elimination. NB this tree wont work for negative stations
 *    or station after km 999.999.999.
 * - starting from the root, we basically have a chain of vettoreCifre but now each entry is a number of the kilometer of the station.
 *    we start from the most significant number to less one. This means the depth of each tree is always fixed:
 *    if the kilometer has 3 numbers (es 123 or 999) daniel'sTree depth will be 4 vettoreCifre;
 *    if the kilometer has 8 number (es 89.789.345) daniel'sTree depth will be 9 vettoreCifre.
 *    this means that in the worst scenario possible i will need only 9 operation to find a station that is present.
 * this tree won't be used to find a given station but to find the neighbour station to a given point (kilometer).
 * given a generic kilometer from 0 to 999.999.999, in the worst case daniel'sTree will find the neighbour station in maximum only 90 operations.
 * daniel'sTree is strongly used into addedMaxAuto() and removedMaxAuto() making those WAY more efficient.
 * it's also used in inserisciStazione() in order to not browse the entire linkedList station.
*
* Visual representation:
 *
 *
 *   sIniziale = 12345                                  nb ash(12345)=436.060; ash(23456)=254.304

  ----------------------------                      *******************************************************
  | STAZIONE:                |                      *                TABLE                                *
  | km 12345                 |                      *-----------------------------------------------------*
  | autoMax 10               |                      * [0]: miaStazione NULL sameHash NULL                 *
  | maxNextStazione 23456    |                      *-----------------------------------------------------*
  | maxPrevStazione NULL     |                      * [1]: miaStazione NULL sameHash NULL                 *
  | nextStazione 23456       |                      *-----------------------------------------------------*
  | preStazione NULL         |                      *                 .                                   *
  | puntDS tab 5.1.2.3.4     |                      *                 .                                   *
  ----------------------------                      *                 .                                   *
                                                    *-----------------------------------------------------*
                                                    * [254304]: miaStazione STAZIONE 23456 sameHash NULL  *
                                                    *-----------------------------------------------------*
  ----------------------------                      *                 .                                   *
  | STAZIONE:                |                      *                 .                                   *
  | km 23456                 |                      *                 .                                   *
  | autoMax 40000            |                      *-----------------------------------------------------*
  | maxNextStazione NULL     |                      * [436060]: miaStazione STAZIONE 12345 sameHash NULL  *
  | maxPrevStazione NULL     |                      *-----------------------------------------------------*
  | nextStazione NULL        |                      *                 .                                   *
  | preStazione 12345        |                      *                 .                                   *
  | puntDS tab 5.2.3.4.5     |                      *                 .                                   *
  ----------------------------                      *******************************************************

    inizio:                                    tab 5:                                tab 5.1:
  |-----------------------------|   *------->|-----------------------------|   *-->|--------------------------------|
  | padre NULL                  |   |        | padre inizio                |   |   | padre tab 5                    |
  | cont 1                      |   |        | cont 2                      |   |   | cont 1                         |
  | c[0].x 0    c[0].nxt NULL   |   |        | c[0].x 0    c[0].nxt NULL   |   |   | c[0].x 0    c[0].nxt NULL      |
  | c[1].x 0    c[1].nxt NULL   |   |        | c[1].x 1    c[1].nxt------------*   | c[1].x 0    c[1].nxt NULL      |
  | c[2].x 0    c[2].nxt NULL   |   |        | c[2].x 1    c[2].nxt------------*   | c[2].x 1    c[2].nxt tab 5.1.2 |
  | c[3].x 0    c[3].nxt NULL   |   |        | c[3].x 0    c[3].nxt NULL   |   |   | c[3].x 0    c[3].nxt NULL      |
  | c[4].x 0    c[4].nxt NULL   |   |        | c[4].x 0    c[4].nxt NULL   |   |   | c[4].x 0    c[4].nxt NULL      |
  | c[5].x 1    c[5].nxt------------*        | c[5].x 0    c[5].nxt NULL   |   |   | c[5].x 0    c[5].nxt NULL      |
  | c[6].x 0    c[6].nxt NULL   |            | c[6].x 0    c[6].nxt NULL   |   |   | c[6].x 0    c[6].nxt NULL      |
  | c[7].x 0    c[7].nxt NULL   |            | c[7].x 0    c[7].nxt NULL   |   |   | c[7].x 0    c[7].nxt NULL      |
  | c[8].x 0    c[8].nxt NULL   |            | c[8].x 0    c[8].nxt NULL   |   |   | c[8].x 0    c[8].nxt NULL      |
  | c[9].x 0    c[9].nxt NULL   |            | c[9].x 0    c[9].nxt NULL   |   |   | c[9].x 0    c[9].nxt NULL      |
  |-----------------------------|            |-----------------------------|   |   |--------------------------------|
                                                                               |
                                                                               *------------------------------------------------------------------*
  tab 5.1.2:                                    tab 5.1.2.3:                                tab 5.1.2.3.4:                                        |
  |-----------------------------|   *------->|-----------------------------|   *-->|-----------------------------|                                |
  | padre tab 5.1               |   |        | padre tab 5.1.2             |   |   | padre tab 5.1.2.3           |                                |
  | cont 1                      |   |        | cont 1                      |   |   | cont 1                      |                                |
  | c[0].x 0    c[0].nxt NULL   |   |        | c[0].x 0    c[0].nxt NULL   |   |   | c[0].x 0    c[0].nxt NULL   |                                |
  | c[1].x 0    c[1].nxt NULL   |   |        | c[1].x 0    c[1].nxt NULL   |   |   | c[1].x 0    c[1].nxt NULL   |                                |
  | c[2].x 0    c[2].nxt NULL   |   |        | c[2].x 0    c[2].nxt NULL   |   |   | c[2].x 0    c[2].nxt NULL   |                                |
  | c[3].x 1    c[3].nxt------------*        | c[3].x 0    c[3].nxt NULL   |   |   | c[3].x 0    c[3].nxt NULL   |                                |
  | c[4].x 0    c[4].nxt NULL   |            | c[4].x 1    c[4].nxt------------*   | c[4].x 0    c[4].nxt NULL   |                                |
  | c[5].x 0    c[5].nxt NULL   |            | c[5].x 0    c[5].nxt NULL   |       | c[5].x 1    c[5].nxt--------------> STAZIONE 12345           |
  | c[6].x 0    c[6].nxt NULL   |            | c[6].x 0    c[6].nxt NULL   |       | c[6].x 0    c[6].nxt NULL   |                                |
  | c[7].x 0    c[7].nxt NULL   |            | c[7].x 0    c[7].nxt NULL   |       | c[7].x 0    c[7].nxt NULL   |                                |
  | c[8].x 0    c[8].nxt NULL   |            | c[8].x 0    c[8].nxt NULL   |       | c[8].x 0    c[8].nxt NULL   |                                |
  | c[9].x 0    c[9].nxt NULL   |            | c[9].x 0    c[9].nxt NULL   |       | c[9].x 0    c[9].nxt NULL   |                                |
  |-----------------------------|            |-----------------------------|       |-----------------------------|                                |
                                                                                                                                                  |
                                                                                                                                                  |
  *-----------------------------------------------------------------------------------------------------------------------------------------------*
  |
  V
  tab 5.2:                                    tab 5.2.3:                                tab 5.2.3.4:
  |-----------------------------|   *------->|-----------------------------|   *-->|------------------------------------|
  | padre tab 5                 |   |        | padre tab 5.2               |   |   | padre tab 5.2.3                    |
  | cont 1                      |   |        | cont 1                      |   |   | cont 1                             |
  | c[0].x 0    c[0].nxt NULL   |   |        | c[0].x 0    c[0].nxt NULL   |   |   | c[0].x 0    c[0].nxt NULL          |
  | c[1].x 0    c[1].nxt NULL   |   |        | c[1].x 0    c[1].nxt NULL   |   |   | c[1].x 0    c[1].nxt NULL          |
  | c[2].x 0    c[2].nxt NULL   |   |        | c[2].x 0    c[2].nxt NULL   |   |   | c[2].x 0    c[2].nxt NULL          |
  | c[3].x 1    c[3].nxt------------*        | c[3].x 0    c[3].nxt NULL   |   |   | c[3].x 0    c[3].nxt NULL          |
  | c[4].x 0    c[4].nxt NULL   |            | c[4].x 1    c[4].nxt------------*   | c[4].x 0    c[4].nxt NULL          |
  | c[5].x 0    c[5].nxt NULL   |            | c[5].x 0    c[5].nxt NULL   |       | c[5].x 1    c[5].nxt tab 5.2.3.4.5 |
  | c[6].x 0    c[6].nxt NULL   |            | c[6].x 0    c[6].nxt NULL   |       | c[6].x 0    c[6].nxt NULL          |
  | c[7].x 0    c[7].nxt NULL   |            | c[7].x 0    c[7].nxt NULL   |       | c[7].x 0    c[7].nxt NULL          |
  | c[8].x 0    c[8].nxt NULL   |            | c[8].x 0    c[8].nxt NULL   |       | c[8].x 0    c[8].nxt NULL          |
  | c[9].x 0    c[9].nxt NULL   |            | c[9].x 0    c[9].nxt NULL   |       | c[9].x 0    c[9].nxt NULL          |
  |-----------------------------|            |-----------------------------|       |------------------------------------|


  tab 5.2.3.4.5:
  |-----------------------------|
  | padre tab 5.2.3.4           |
  | cont 1                      |
  | c[0].x 0    c[0].nxt NULL   |
  | c[1].x 0    c[1].nxt NULL   |
  | c[2].x 0    c[2].nxt NULL   |
  | c[3].x 0    c[3].nxt NULL   |
  | c[4].x 0    c[4].nxt NULL   |
  | c[5].x 0    c[5].nxt NULL   |
  | c[6].x 1    c[6].nxt--------------> STAZIONE 23456
  | c[7].x 0    c[7].nxt NULL   |
  | c[8].x 0    c[8].nxt NULL   |
  | c[9].x 0    c[9].nxt NULL   |
  |-----------------------------|

*
* Heap memory is block allocated. My functions dammiMemoria() will call malloc if we need extra space.
*/

#include <stdio.h>
#include <stdlib.h>

#define dimHashTable 100000
#define dimVettoreTappe 1000
//using specific numbers to reduce fragmentation
#define dimMallocVettori 25600 //I will use a multiple of 256 (sizeof(vettoreCife)*256=multiple of 4096=1 memory page)
#define dimMallocStazioni 51200 //I will use a multiple of 512 (sizeof(stazione)*512=multiple of 4096=1 memory page)
#define dimMallocBuckets 256000 //I will use a multiple of 256 (sizeof(nodoHashTable)*256=multiple of 4096=1 memory page)

typedef struct stazione{
    int km;
    int parcheggio[512];
    int indice;
    int autoMax;
    struct stazione *maxNextStazione;
    struct stazione *maxPreStazione;
    struct stazione *nextStazione;
    struct stazione *preStazione;
    struct vettoreCifre *puntDS;
} stazione;

typedef struct nodoHashTable {
    stazione *miaStazione;
    struct nodoHashTable *sameAsh;
} nodoHashTable;

typedef struct cifra{
    unsigned x:1;
    void * nxt;
} cifra;

typedef struct vettoreCifre{
    cifra c[10];
    struct vettoreCifre * padre;
    char cont;
} vettoreCifre;

//variabili globali
nodoHashTable table[dimHashTable]; //HashTable

char stringa[10]; //internally used by DS
vettoreCifre inizio; //daniel'sTree root

int tappe[dimVettoreTappe]; //array used to store journey's steps
int indiceTappe = 0;

stazione *sIniziale = NULL; //linked list start point

//malloc support for Nodes(stazioni)
int contatore = 0;
stazione *inizioMemNodi = NULL;
stazione *fineMemNodi = NULL;
stazione *sonoAllaFine = NULL;

//malloc support for Buckets
int contatore2 = 0;
nodoHashTable *inizioMemBuckets = NULL;
nodoHashTable *fineMemBuckets = NULL;
nodoHashTable *sonoAllaFine2 = NULL;

//malloc support for daniel'sTree
int contatore3 = 0;
vettoreCifre * inizioMemVettori = NULL;
vettoreCifre * fineMemVettori = NULL;
vettoreCifre * sonoAllaFine3 = NULL;
//fine variabili globali

//functions declaration
void gestioneFlussoEsecuzione();
void initializeTable();
void initializeTappe();
int inizializzaVettore(vettoreCifre* v);
int inizializzaStringa();
int aggiungiStazione(int kilometro);
stazione *dammiMemoriaNodi();
void initializeStation(stazione* s);
int insertIntoTable(int kilometro, stazione* newStaz);
int ash(int km);
nodoHashTable *dammiMemoriaBuckets();
void initializeBucket(nodoHashTable* n);
int aggiungiStazioneDS(int kilometro, stazione* s);
int nCifre(int n);
int aggiornaStringa(int km, int ncifre);
int ricercaDS(vettoreCifre * v, int cifra, int step, stazione* s);
int ultimoStepAggStaz(vettoreCifre * v, int cifra, stazione* s);
int creaDS(vettoreCifre * v, int cifra, int step, stazione* s);
vettoreCifre * dammiMemoriaVettori();
int cercaInTab(int kilometro);
int aggiornaDipendenze(stazione* staz);
int aggiornaMaxNext(stazione* arrivo, stazione* partenza);
int aggiornaMaxPrev(stazione* arrivo, stazione* partenza);
stazione * ricercaVicinoDS(int kilometro);
stazione * dammiStazione(int kilometro);
stazione * ricercaVicinoDSVera(int kilometro, vettoreCifre * v, int cifra, int step);
stazione *ricercaMaxDS(vettoreCifre *v, int c);
stazione *ricercaMinDS(vettoreCifre *v, int c);
int aggiungiAuto(int kmStazione, int autonomia, int stampa);
int addedMaxAuto(stazione * staz, int autonomia);
int rimuoviAuto(int kmStazione, int autonomia);
int calcolaNuovaAutonomia(stazione* staz, int oldMax);
int removedMaxAuto(stazione * staz, int oldAutonomia);
int aggiornaMaxPrevStaz(stazione* partenza, stazione* arrivo);
int aggiornaMaxNextStaz(stazione* partenza, stazione* arrivo);
int rimuoviStazione(int kilometro);
int eliminaDS(stazione* s);
int riparaEliminazione(vettoreCifre *v, int k, int cifre);
int rimuoviNodoAshTable(int kilometro);
int rimuoviNodoLista(stazione* staz);
int pianificaPercorso(int partenza, int arrivo);
int pianificaAvanti(stazione *partenza, stazione *arrivo, int veroArrivo);
int inserisciTappa(int k);
int piccoloPianificaAvanti(stazione *partenza, stazione *arrivo, int veroInizio);
void stampaTappe();
int pianificaIndietro(stazione* partenza, stazione* arrivo, int veroArrivo);
int piccoloPianificaIndietro(stazione* partenza, stazione* arrivo, int veroInizio);
int trovaAlternative();
int maxPrevStaz(stazione * s);
void swapTappe(int indice, int km);
int trovaAlternativeTest();
int trovaAlternativeInfame();
stazione* trovaStazioneInfame(stazione* trov, stazione* min, stazione* max);
//end of function declaration

int main() {
    gestioneFlussoEsecuzione();
    return 0;
}

void gestioneFlussoEsecuzione() {
    initializeTable();
    initializeTappe();
    inizializzaVettore(&inizio);
    inizializzaStringa();
    char c, Kstaz;
    int i, supp, Staz;
    while ((c = getchar()) != EOF) {
        switch (c) {
            case 'a':
                for (i = 0; i < 9; i++) {//capiamo cosa aggiunge
                    c = getchar();
                }
                switch (c) {
                    case 's':
                        for (i = 0; i < 9; i++) {//capiamo a che chilometro va aggiunta
                            Kstaz = getchar();
                        }//ora c=kilometro a cui aggiungere la stazione
                        Staz = Kstaz - '0';
                        while ((c = getchar()) != ' ' && c != '\n') {
                            Staz = Staz * 10 + (c - '0');
                        }
                        if(aggiungiStazione(Staz) != 2){//stazione non gia presente
                            //ora sono arrivato a leggere il carattere successivo al kilometro a cui
                            // aggingere la stazione: puo essere BLANK (=ci sono auto da aggiungere) oppure \n (fine comando-auto da aggiungere)
                            if (c == ' ') {
                                c = getchar();
                                if ((int) c == 0) {//aggiungo stazione con zero auto
                                    c = getchar();//rimuovo il \n finale della riga
                                } else {//c'è almeno 1 auto da aggiungere
                                    while ((c = getchar()) != ' ' && c != '\n') {
                                        continue;//non ci importa il numero di auto, ma solo la loro autonomia (in realta
                                        //mi interessa solo l'autonomia dell auto migliore, ma dato che potrebbe essere rimossa devo
                                        //salvarmele tutte
                                    }//ora c=primo valore del autonomia della prima auto
                                    while (c != '\n') {
                                        c = getchar();
                                        supp = c - '0';
                                        while ((c = getchar()) != ' ' && c != '\n') {
                                            supp = supp * 10 + (c - '0');
                                        }
                                        aggiungiAuto(Staz, supp, 0);
                                    }
                                }
                            }
                        }else{//non mi importa della riga, scorro fino alla fine
                            while((c = getchar()) != '\n'){
                                continue;
                            }
                        }
                        break;
                    case 'a'://c=='a' comando AGGIUNGI-AUTO
                        for (i = 0; i < 5; i++) {//capiamo in che stazione va aggiunta
                            Kstaz = getchar();//Kstaz è il kilometro (stazione) alla quale aggiugere l'auto
                        }
                        Staz = Kstaz - '0';
                        while ((c = getchar()) != ' ' && c != '\n') {
                            Staz = Staz * 10 + (c - '0');
                        }//ora Staz è il chilometro a cui aggiungere le auto. c=spazio vuoto oppure \n
                        while (c != '\n') {
                            c = getchar();//c = primo valore autonomia prima auto
                            supp = c - '0';
                            while ((c = getchar()) != ' ' && c != '\n') {
                                supp = supp * 10 + (c - '0');
                            }
                            aggiungiAuto(Staz, supp, 1);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case 'r':
                for (i = 0; i < 13; i++) {//
                    Kstaz = getchar();//Kstaz è il primo valore del kilometro (stazione) alla quale rottamare l'auto
                }
                Staz = Kstaz - '0';
                while ((c = getchar()) != ' ' && c != '\n') {
                    Staz = Staz * 10 + (c - '0');
                }//Staz= kilometro(stazione) alla quale rimuovere le auto
                while (c != '\n') {
                    c = getchar();//c = primo valore autonomia prima auto
                    supp = c - '0';
                    while ((c = getchar()) != ' ' && c != '\n') {
                        supp = supp * 10 + (c - '0');
                    }//supp = autonomia corretta auto
                    rimuoviAuto(Staz, supp);//c è l autonomia dell'auto da rimuovere
                }
                break;
            case 'd':
                for (i = 0; i < 19; i++) {//
                    Kstaz = getchar();//Kstaz è il primo valore del kilometro (stazione) alla quale rottamare l'auto
                }
                Staz = Kstaz - '0';
                while ((c = getchar()) != ' ' && c != '\n') {
                    Staz = Staz * 10 + (c - '0');
                }//Staz= kilometro(stazione) da rimuovere
                rimuoviStazione(Staz);
                while (c != '\n') {//in teoria superfluo, arrivo fino a fine riga
                    c = getchar();
                }
                break;
            case 'p':
                for (i = 0; i < 19; i++) {
                    c = getchar();
                }//riuso impropriamente le variabili Staz e supp disponibili: c = primo carattere numero kilometro stazione partenza
                Staz = c - '0';
                while ((c = getchar()) != ' ' && c != '\n') {
                    Staz = Staz * 10 + (c - '0');
                }//Staz= kilometro(stazione) da rimuovere
                c = getchar();//c = primo valore numero kilometro stazione arrivo
                supp = c - '0';
                while ((c = getchar()) != '\n') {
                    supp = supp * 10 + (c - '0');
                }//supp = kilometro stazione di arrivo
                pianificaPercorso(Staz, supp);
                break;

            default:
                break;
        }
    }
}

void initializeTable() {//initialize HashTable
    int i;
    for (i = 0; i < dimHashTable; i++) {
        table[i].miaStazione = NULL;
        table[i].sameAsh = NULL;
    }
}

void initializeTappe(){//initialize tappe and indiceTappe
    int i;
    for(i = 0; i < dimVettoreTappe; i++){
        tappe[i] = -1;
    }
    indiceTappe = 0;
}

int inizializzaVettore(vettoreCifre* v){//initialize given vettoreCifre
    int i = 0;
    for(i=0; i<10; i++){
        v->c[i].x = 0;
        v->c[i].nxt = NULL;
        v->padre = NULL;
        v->cont = (char)0;
    }
    return 1;
}

int inizializzaStringa(){//initialize stringa
    stringa[0] = (char) 63;
    stringa[1] = (char) 63;
    stringa[2] = (char) 63;
    stringa[3] = (char) 63;
    stringa[4] = (char) 63;
    stringa[5] = (char) 63;
    stringa[6] = (char) 63;
    stringa[7] = (char) 63;
    stringa[8] = (char) 63;
    stringa[9] = (char) 63;
    return 1;
}
/**
 * creates a new station at the given kilometer (if not already present), then asks to insert the new station
 * into the HashTable and daniel'sTree
 * @param kilometro kilometer of the station to be added
 * @return 2 if not added, 1 if added, 0 if something very bad happened
 */
int aggiungiStazione(int kilometro){
    stazione *s = NULL;
    stazione *support = NULL;
    if (sIniziale == NULL) {
        //printf("aggiungo stazione al kilometro %d\n", kilometro);
        printf("aggiunta\n");
        sIniziale = dammiMemoriaNodi();
        initializeStation(sIniziale);
        sIniziale->km = kilometro;
        insertIntoTable(kilometro, sIniziale);
        aggiungiStazioneDS(kilometro, sIniziale);
        return 1;
    } else {//esiste almeno 1 stazione
        if (cercaInTab(kilometro)) {//trovata
            //printf("stazione gia presente: impssibile aggiungere stazione al kilometro %d\n", kilometro);
            printf("non aggiunta\n");
            return 2;
        } else {//non trovata: ok possiamo aggiungerla
            //printf("aggiungo stazione al kilometro %d\n", kilometro);
            printf("aggiunta\n");
            s = sIniziale;
            support = dammiMemoriaNodi();
            initializeStation(support);
            support->km = kilometro;
            if (s->km > kilometro) {//inserimento all inizio lista (1 posto)
                aggiungiStazioneDS(kilometro, support);
                support->nextStazione = (struct stazione *) s;
                s->preStazione = (struct stazione *) support;
                sIniziale = support;
                insertIntoTable(kilometro, support);
                aggiornaDipendenze(support);
                return 1;
            }//non va inserita in cima alla lista: o è in mezzo o all ultimo posto
            if (s->nextStazione == NULL) {//inserimento all'ultimo posto
                aggiungiStazioneDS(kilometro, support);
                s->nextStazione = (struct stazione *) support;
                support->preStazione = (struct stazione *) s;
                insertIntoTable(kilometro, support);
                aggiornaDipendenze(support);
                return 1;
            }
            s = ricercaVicinoDS(kilometro);
            if (s->km > kilometro) {//va attaccato prima di s
                aggiungiStazioneDS(kilometro, support);
                ((stazione *) (s->preStazione))->nextStazione = (struct stazione *) support;
                support->preStazione = s->preStazione;
                support->nextStazione = (struct stazione *) s;
                s->preStazione = (struct stazione *) support;
                insertIntoTable(kilometro, support);
                aggiornaDipendenze(support);
                return 1;
            }else{//va attaccato dopo s
                if (s->nextStazione == NULL) {//inserimento all'ultimo posto
                    aggiungiStazioneDS(kilometro, support);
                    s->nextStazione = (struct stazione *) support;
                    support->preStazione = (struct stazione *) s;
                    insertIntoTable(kilometro, support);
                    aggiornaDipendenze(support);
                    return 1;
                }
                aggiungiStazioneDS(kilometro, support);
                ((stazione *) (s->nextStazione))->preStazione = (struct stazione *) support;
                support->nextStazione = s->nextStazione;
                support->preStazione = (struct stazione *) s;
                s->nextStazione = (struct stazione *) support;
                insertIntoTable(kilometro, support);
                aggiornaDipendenze(support);
                return 1;
            }
        }
    }
}

/**
 * @return pointer to a fresh memory where i can store a station
 */
stazione *dammiMemoriaNodi() {
    if (inizioMemNodi == NULL) {//primo nodo in assoluto
        //printf("creo spazio di memoria\n");
        if ((inizioMemNodi = (stazione *) malloc(dimMallocStazioni * sizeof(stazione))) != NULL) {
            fineMemNodi = inizioMemNodi + dimMallocStazioni * sizeof(stazione);
            sonoAllaFine = inizioMemNodi;
            contatore = 0;
            return &inizioMemNodi[contatore];
        } else { printf("errore creazione spazio memoria aggiuntivo peer nodi\n"); }
    } else {
        sonoAllaFine = sonoAllaFine + sizeof(stazione);
        if (sonoAllaFine >= fineMemNodi) {//ho finito la memoria fornita
            //printf("ho finito la memoria fornita\n");
            if ((inizioMemNodi = (stazione *) malloc(dimMallocStazioni * sizeof(stazione))) != NULL) {
                fineMemNodi = inizioMemNodi + dimMallocStazioni * sizeof(stazione);
                sonoAllaFine = inizioMemNodi;
                contatore = 0;
                return &inizioMemNodi[contatore];
            } else { printf("errore creazione spazio memoria aggiuntivo\n"); }
        }
    }
    contatore = contatore + 1;
    return &inizioMemNodi[contatore];
}

void initializeStation(stazione* s) {//initialize given station
    int i;
    s->km = -1;
    for (i = 0; i < 512; i++) {
        s->parcheggio[i] = -1;
    }
    s->autoMax = 0;
    s->indice = 0;
    s->maxNextStazione = NULL;
    s->maxPreStazione = NULL;
    s->nextStazione = NULL;
    s->preStazione = NULL;
    s->puntDS = NULL;
}

/**
 * this functions makes the given station accessible from the HashTable
 * @param kilometro kilometer of the station to be added
 * @param newStaz pointer to the station to be added
 * @return 1 if the add is succesfull, 0 if something unexpected happens
 */
int insertIntoTable(int kilometro, stazione* newStaz){
    nodoHashTable * puntNodoTab = NULL;
    puntNodoTab = &table[ash(kilometro)];
    if(puntNodoTab->miaStazione == NULL){//inserimento diretto nella table
        puntNodoTab->miaStazione = newStaz;
        return 1;
    }else{
        while(puntNodoTab->sameAsh != NULL){
            puntNodoTab = puntNodoTab->sameAsh;
        }
        puntNodoTab->sameAsh = (struct nodoHashTable *)dammiMemoriaBuckets();
        initializeBucket(puntNodoTab->sameAsh);
        ((puntNodoTab->sameAsh))->miaStazione = newStaz;
        return 1;
    }
    return 0;//technically, we should never reach this point
}

/**
 * @param km km of the station
 * @return entry of the HashTable
 */
int ash(int km) {
    unsigned long n; //for stations above half million kilometers, using basic integer will result in overflow and wrong results
    n = (unsigned long) km;
    n = (n * (n + 3)) % dimHashTable; //using Knuth's ash function
    km = (int) n;
    return km;
}

/**
 * @return pointer to a fresh memory where i can store a nodoHashTable
 */
nodoHashTable *dammiMemoriaBuckets() {
    if (inizioMemBuckets == NULL) {//primo nodo in assoluto
        //printf("creo spazio di memoria\n");
        if ((inizioMemBuckets = (nodoHashTable *) malloc(dimMallocBuckets * sizeof(nodoHashTable))) != NULL) {
            fineMemBuckets = inizioMemBuckets + dimMallocBuckets * sizeof(nodoHashTable);
            sonoAllaFine2 = inizioMemBuckets;
            contatore2 = 0;
            return &inizioMemBuckets[contatore2];
        } else { printf("errore creazione spazio memoria aggiuntivo peer nodi\n"); }
    } else {
        sonoAllaFine2 = sonoAllaFine2 + sizeof(nodoHashTable);
        if (sonoAllaFine2 >= fineMemBuckets) {//ho finito la memoria fornita
            //printf("ho finito la memoria fornita\n");
            if ((inizioMemBuckets = (nodoHashTable *) malloc(dimMallocBuckets * sizeof(nodoHashTable))) != NULL) {
                fineMemBuckets = inizioMemBuckets + dimMallocBuckets * sizeof(nodoHashTable);
                sonoAllaFine2 = inizioMemBuckets;
                contatore2 = 0;
                return &inizioMemBuckets[contatore2];
            } else { printf("errore creazione spazio memoria aggiuntivo\n"); }
        }
    }
    contatore2 = contatore2 + 1;
    return &inizioMemBuckets[contatore2];
}

void initializeBucket(nodoHashTable* n){//initialize given nodoHashTable
    n->miaStazione = NULL;
    n->sameAsh = NULL;
}

/**
 * this function insert the given station (pointer to that) into daniel'sTree
 * @param kilometro kilometer of the station to add
 * @param s pointer to the station to add
 * @return
 */
int aggiungiStazioneDS(int kilometro, stazione* s){
    int km = kilometro;
    int c = 0;
    inizializzaStringa();
    c = nCifre(km);
    aggiornaStringa(km, c);
    return ricercaDS(&inizio, c, c, s);
}

/**
 * @param n integer
 * @return number of character of n [x10 BASE 10]
 */
int nCifre(int n){
    int i = 1;
    while(n >= 10){
        n = n / 10;
        i++;
    }
    return i;
}

/**
 * this function fills an array "backwards" with given number.
 * @param km integer rapresenting kilometer
 * @param ncifre number of digints of km in base 10
 * @return 1
 * ES: if km=12345 ----->stringa[0]=5, stringa[1]=4 .... ""stringa=54321""
 */
int aggiornaStringa(int km, int ncifre){
    int i = 0;
    for(i=0; i<ncifre; i++){
        stringa[i] = km%10;
        km = km/10;
    }
    return 1;
}

/**
 * this function browse daniel'sTree to find the right spot to memorize a new added station (can be recursive)
 * @param v current node of the tree
 * @param cifra current digit of the kilometer that we are checking
 * @param step number of steps left to reach leafs
 * @param s pointer to the station to be added
 * @return 1
 */
int ricercaDS(vettoreCifre * v, int cifra, int step, stazione* s){
    if(step == 0){
        ultimoStepAggStaz(v, cifra, s);
    }else{
        if(v->c[cifra].x == 1){//presente
            return ricercaDS(v->c[cifra].nxt, stringa[step-1], step-1, s);
        }else{//non presente, inzia a creare
            return creaDS(v, cifra, step, s);
        }
    }
    return 0;//should never reach this branch
}

/**
 * this function attaches new station to daniel'sTree's leaf-node
 * @param v current leaf node of the tree
 * @param cifra last digit of the km (es km=1234 ----> cifra=4)
 * @param s pointer to the station to be added
 * @return 1
 */
int ultimoStepAggStaz(vettoreCifre * v, int cifra, stazione* s){
    v->c[cifra].nxt = (stazione *) s;
    v->c[cifra].x = 1;
    v->cont = (char)((int)v->cont + 1);
    s->puntDS = v;
    return 1;
}

/**
 * this function is responsible for the creation of daniel'sTree's nodes (vettoreCifra). It can call itself recursively
 * @param v current node
 * @param cifra current digit number
 * @param step current step
 * @param s pointer to the station
 * @return 1, 0 if something wrong happened
 */
int creaDS(vettoreCifre * v, int cifra, int step, stazione* s){
    if(step == 0){
        ultimoStepAggStaz(v, cifra, s);
    }else{
        v->c[cifra].nxt = (vettoreCifre *)dammiMemoriaVettori();
        inizializzaVettore(v->c[cifra].nxt);
        v->c[cifra].x = 1;
        v->cont = (char)((int)v->cont + 1);
        ((vettoreCifre* )(v->c[cifra].nxt))->padre = v;
        return creaDS(v->c[cifra].nxt, stringa[step-1], step-1, s);
    }
    return 0;//should never reach this branch
}

/**
 * @return pointer to a fresh memory where i can store a daniel'sTree's node
 */
vettoreCifre * dammiMemoriaVettori(){
    if(inizioMemVettori == NULL){
        if((inizioMemVettori = (vettoreCifre *) malloc(dimMallocVettori * sizeof (vettoreCifre)))!=NULL){
            fineMemVettori = inizioMemVettori + dimMallocVettori * sizeof (vettoreCifre);
            sonoAllaFine3 = inizioMemVettori;
            contatore3 = 0;
            return &inizioMemVettori[contatore3];
        }else{
            printf("errore creazione spazio aggiuntivo vettori\n");
        }
    }else{
        sonoAllaFine3 = sonoAllaFine3 + sizeof (vettoreCifre);
        if(sonoAllaFine3 >= fineMemVettori){
            if((inizioMemVettori = (vettoreCifre *) malloc(dimMallocVettori * sizeof (vettoreCifre)))!=NULL){
                fineMemVettori = inizioMemVettori + dimMallocVettori * sizeof (vettoreCifre);
                sonoAllaFine3 = inizioMemVettori;
                contatore3 = 0;
                return &inizioMemVettori[contatore3];
            }else{
                printf("errore creazione spazio aggiuntivo vettori\n");
            }
        }
    }
    contatore3 = contatore3 + 1;
    return &inizioMemVettori[contatore3];
}

/**
 * @param kilometro of the station to be searched
 * @return 1 if the station is present, 0 otherwise
 */
int cercaInTab(int kilometro){
    nodoHashTable * y = NULL;
    y = &table[ash(kilometro)];
    while(y->sameAsh != NULL){
        if(y->miaStazione->km == kilometro){//trovata
            return 1;
        }
        y = (nodoHashTable *) y->sameAsh;
    }
    if(y->miaStazione != NULL){
        if(y->miaStazione->km == kilometro){
            return 1;
        }
    }
    return 0;//non trovata
}

/**
 * this function sets maxNextStazione and maxPreStazione for the freshly added station
 * @param staz freshly added station
 * @return 1
 */
int aggiornaDipendenze(stazione* staz){//this function can be void
    //configuring MaxNext
    if(staz->nextStazione != NULL){
        if(staz->nextStazione->maxNextStazione != NULL){//next station can be reached backwards from someone
            aggiornaMaxNext(staz,  staz->nextStazione->maxNextStazione);//check if that someone can reach me (staz) too
        }else{//next station cant be reached from anyone backwords: this means the only one station that can reach my staz backwords CAN (not mandatory) is the next one
            aggiornaMaxNext(staz,  staz->nextStazione);
        }
    }//if staz->nextStazione == NULL: staz is the last station: for sure there isn't an other station after that that can reach our freshly added (last) station
    //configuring MaxPrev
    if(staz->preStazione != NULL){
        if(staz->preStazione->maxPreStazione != NULL){//prev station can be reached from someone behind
            aggiornaMaxPrev(staz, staz->preStazione->maxPreStazione);//check if that someone can reach me (staz) too
        }else{//prev station cant be reached from anyone: this means the only one station that can reach my staz CAN (not mandatory) is the previous one
            aggiornaMaxPrev(staz, staz->preStazione);
        }
    }//if staz->preStazione == NULL: staz is the first station: for sure there isnt an other station before that that can reach our freshly added (first) station
    return 1;
}

/**
 * support for aggiornaDipendenze()
 * @param arrivo new station
 * @param partenza possible maxNextStazione for my new station
 * @return 1 if we found at least 1 bigger station that can reach arrivo backwards. 0 otherwise
 */
int aggiornaMaxNext(stazione* arrivo, stazione* partenza){
    //backward direction of march
    if(arrivo != partenza){
        if((partenza->km - arrivo->km) <= partenza->autoMax){
            arrivo->maxNextStazione =  partenza;
            return 1;
        }else{
            if(partenza->preStazione!=NULL){
                return aggiornaMaxNext(arrivo, partenza->preStazione);
            }
            return 1;
        }
    }else{//arrivo == partenza
        //arrivo->maxNextStazione = NULL;
        return 0;
    }
}

/**
 * support for aggiornaDipendenze()
 * @param arrivo new station
 * @param partenza possible maxPrevStazione for my new station
 * @return 1 if we found at least 1 smaller station that can reach arrivo. 0 otherwise
 */
int aggiornaMaxPrev(stazione* arrivo, stazione* partenza){
    //straight direction of march
    if(arrivo != partenza){
        if((arrivo->km - partenza->km) <= partenza->autoMax){
            arrivo->maxPreStazione = partenza;
            return 1;
        }else{
            if(partenza->nextStazione!=NULL){
                return aggiornaMaxPrev(arrivo,  partenza->nextStazione);
            }
            return 1;
        }
    }else{//partenza == arrivo
        //arrivo->maxPreStazione = NULL;
        return 0;
    }
}

/**
 * @param kilometro
 * @return:
 *  - a pointer to the station if there's one at given km
 *  - a pointer to one of the neighbours station to a given km if there is no station at given km
 *  - NULL if the highway is empty
 */
stazione * ricercaVicinoDS(int kilometro){//always returns a station! wrost case: unica stazione esistente
    int km = kilometro;
    stazione * trovata = NULL;
    int c = 0;
    inizializzaStringa();
    c = nCifre(km);
    aggiornaStringa(km, c);
    if((trovata = dammiStazione(km))!=NULL){
        return trovata;
    }else{
        if(sIniziale != NULL){
            return ricercaVicinoDSVera(km, &inizio, c, c);
        }else{
            return NULL;//ok ancora consentito sia null
        }
    }
}

/**
 * search for station at given km. Uses HashTable
 * @param kilometro of the station
 * @return pointer to searched station if present, NULL otherwise
 */
stazione * dammiStazione(int kilometro){
    nodoHashTable* y = NULL;
    y = &table[ash(kilometro)];
    while(y->sameAsh != NULL){
        if(y->miaStazione->km == kilometro){
            return y->miaStazione;
        }
        y =  y->sameAsh;
    }
    if(y->miaStazione != NULL){
        if(y->miaStazione->km == kilometro){
            return y->miaStazione;
        }
    }
    return NULL;
}

/**
 * used by ricercaVicino(). Can call itself recursively.
 * @param kilometro
 * @param v current daniel'sTree's node
 * @param cifra current km's digit
 * @param step current step = (1-trees's depth)     [trees's depth = 1-step]
 * @return pointer to a station, NULL if something very bad happened (like broken data structure)
 */
stazione * ricercaVicinoDSVera(int kilometro, vettoreCifre * v, int cifra, int step){
    int i = 0;
    if(step == 0){
        if(v->c[cifra].x == 1){
            printf("errore! stai cercando una stazione vicina ad un kilometro in cui esiste esattmente una stazione! [ricercaVicinoDSVera]\n");
        }
        for(i=cifra-1; i>=0; i--){
            if(v->c[i].x == 1){
                return v->c[i].nxt;
            }
        }
        for(i=cifra+1; i<10; i++){
            if(v->c[i].x == 1){
                return v->c[i].nxt;
            }
        }
        printf("errore, non dovrei essere qui! [ricercaVicinoDSVera]\n");
        // return 0; should never reach this branch
        return NULL;
    }else{
        if(v->c[cifra].x == 1){
            return ricercaVicinoDSVera(kilometro, v->c[cifra].nxt, stringa[step-1], step-1);
        }else{
            for(i=cifra-1; i>=0; i--){
                if(v->c[i].x == 1){//scelta stazione piu grande tra le minori del km
                    if(v->padre == NULL){//sono al inzio: significa che cambia il numero di cifre, quindi la profondita dell albero, quindi gli step da fare
                        return ricercaMaxDS(v->c[i].nxt, step-(cifra-i+1));
                    }
                    return ricercaMaxDS(v->c[i].nxt, step-1);
                }
            }//non esiste minore
            for(i=cifra+1; i<10; i++){
                if(v->c[i].x == 1){//scelta stazione piu piccola tra le maggiori del km
                    if(v->padre == NULL){//sono al inzio: significa che cambia il numero di cifre, quindi la profondita dell albero, quindi gli step da fare
                        return ricercaMinDS(v->c[i].nxt, step+(i-cifra-1));
                    }
                    return ricercaMinDS(v->c[i].nxt, step-1);
                }
            }
        }
        printf("errore, non dovrei essere qui! [ricercaVicinoDSVera]\n");
        // return 0; should never reach this branch
        return NULL;
    }
}

/**
 * this function returns the max station stored in daniel'sTree starting from given node. Can call itself recursively.
 * @param v current daniel'sTree's node (given node)
 * @param c step remaining
 * @return pointer to a station or NULL if something very bad happened.
 */
stazione *ricercaMaxDS(vettoreCifre *v, int c) {
    int i = 0;
    if (c == 0) {
        for (i = 9; i >= 0; i--) {
            if (v->c[i].x == 1) {
                return v->c[i].nxt;
            }
        }
    } else {
        for (i = 9; i >= 0; i--) {
            if (v->c[i].x == 1) {
                return ricercaMaxDS(v->c[i].nxt, c - 1);
            }
        }
    }
    printf("errore, non dovrei essere qui! [ricercaMaxDS]\n");
    // return 0; should never reach this branch
    return NULL;
}

/**
 * this function returns the minimum station stored in daniel'sTree starting from given node. Can call itself recursively.
 * @param v current daniel'sTree's node (given node)
 * @param c step remaining
 * @return pointer to a station or NULL if something very bad happened.
 */
stazione *ricercaMinDS(vettoreCifre *v, int c) {
    int i = 0;
    if (c == 0) {
        for (i = 0; i < 10; i++) {
            if (v->c[i].x == 1) {
                return v->c[i].nxt;
            }
        }
    } else {
        for (i = 0; i < 10; i++) {
            if (v->c[i].x == 1) {
                return ricercaMinDS(v->c[i].nxt, c - 1);
            }
        }
    }
    printf("errore, non dovrei essere qui! [ricercaMinDS]\n");
    // return 0; should never reach this branch
    return NULL;
}

//end of aggiungiStazione Utils

/**
 * this function adds given car to given station
 * @param kmStazione km of the station where we want to add a car
 * @param autonomia autonomy of new car to add
 * @param stampa 1 if called from aggiungi-auto request, 0 if called from reading a line aggiungi-stazione with cars attached
 * @return 1 if succesfully added to park slot, 0 otherwise (station not available or full park slot)
 */
int aggiungiAuto(int kmStazione, int autonomia, int stampa){//stampa == 1 only if the function is called from a aggiungi-auto request.
    //stampa == 0 if the function is calleed from a aggiungi-stazione request
    stazione* s = NULL;
    s = dammiStazione(kmStazione);
    if(s == NULL){
        //printf("impossibile aggiungere automobile con autonomia %d: la stazione al kilometro %d non esiste\n", autonomia, kmStazione);
        if(stampa) printf("non aggiunta\n");
        return 0;
    }else{
        if(s->indice < 512){//check if there's available space in parking slot
            //printf("aggiungo auto con autonomia %d al kilometro %d\n", autonomia, kmStazione);
            if(stampa) printf("aggiunta\n");
            s->parcheggio[s->indice] = autonomia;
            s->indice++;
            if(autonomia > s->autoMax){
                s->autoMax = autonomia;
                addedMaxAuto(s, autonomia);
            }
            return 1;
        }//we reach this point if there no space for more auto
        //printf("impossibile aggiungere automobile con autonomia %d: il kilometro %d ha il parcheggio pieno\n", autonomia, kmStazione);
        if(stampa)printf("non aggiunta\n");
    }
    return 0;//reaching this point only of the park slot is full
}

/**
 * this function is called when a top car is added to a station. It does not affect the station itself:
 *  we just verify if now we can reach more station than before and in this can we proceed to set
 *  maxNextStazione and maxPreStazione for now reachable stations.
 * @param staz station where we added a new top cra
 * @param autonomia autonomy of the new top car
 * @return 1, 0 if something went wrong
 */
int addedMaxAuto(stazione * staz, int autonomia){
    //conf iniziale
    int x, y, z;
    stazione * miaStaz = NULL;
    stazione * precedenti = NULL;
    stazione * successive = NULL;
    miaStaz = staz;
    z = staz->km;
    x = z + autonomia;
    y = z- autonomia;
    if(x>999999999) x=999999999;
    if(y<0) y=0;
    successive = ricercaVicinoDS(x);
    precedenti = ricercaVicinoDS(y);

    //if(precedenti == NULL || successive == NULL) return 2;// (if no station is available how can we add a car??)

    //ricerca vicino ritorna, dato un km una stazione vicina. ma non sappiamo a priori se e maggiore o minore del kilometro.
    if(successive->km > x) successive = successive->preStazione;
    if(precedenti->km < y) precedenti = precedenti->nextStazione;
    //fine conf iniziale
    while(successive != miaStaz){//parto dal fondo(massimo punto a cui arrivo) a vado verso di me
        if(successive->maxPreStazione != NULL){
            if(successive->maxPreStazione->km <= z) break;//se una stazione sa che una precedente a me arriva a lui, allora lo sanno
            //pure tutte quelle tra me e la stazione. posso uscire.
        }
        successive->maxPreStazione = miaStaz;
        successive = successive->preStazione;
    }
    while(precedenti != miaStaz){//parto dal inizio(massimo punto a cui arrivo) e vado verso di me
        if(precedenti->maxNextStazione != NULL){
            if(precedenti->maxNextStazione->km >= z) break;//se una stazione sa che una successiva a me arriva a lui, allora lo sanno
            //pure tutte quelle tra me e la stazione. posso uscire.
        }
        precedenti->maxNextStazione = miaStaz;
        precedenti = precedenti->nextStazione;
    }
    return 1;
}

//end of aggiungiAuto Utils

/**
 *
 * @param kmStazione km of the station where we need to remove a car
 * @param autonomia autonomy of the car to remove
 * @return 1, 0 if went wrong
 */
int rimuoviAuto(int kmStazione, int autonomia){
    int i, j;
    stazione* s = NULL;
    s = dammiStazione(kmStazione);
    if(s == NULL){
        //printf("impossibile rimuovere auto con autonomia %d , stazione al kilometro %d inesistente\n", autonomia, kmStazione);
        printf("non rottamata\n");
    }else{
        for(i=0; i<s->indice; i++){
            if(s->parcheggio[i] == autonomia){//trovata
                s->indice--;
                for(j=i; j<s->indice; j++){//shifting everything to sx
                    s->parcheggio[j] = s->parcheggio[j+1];
                }
                s->parcheggio[s->indice] = -1;//initializing liberated cell (probably useless)
                if(autonomia == s->autoMax){
                    s->autoMax = calcolaNuovaAutonomia(s, autonomia);
                    if(autonomia != s->autoMax){//rimossa l auto max ed era l unica
                        removedMaxAuto(s, autonomia);
                    }
                }
                //printf("rimossa auto con autonomia %d al kilometro %d\n", autonomia, kmStazione);
                printf("rottamata\n");
                return 1;
            }
        }
        //printf("impossibile rimuovere auto con autonomia %d alla stazione %d: l'auto non è presente\n", autonomia, kmStazione);
        printf("non rottamata\n");
    }
    return 0;//return here if either there is no station or no car
}

/**
 * @param staz pointer to station where we removed a car
 * @param oldMax autonomy of the top car removed
 * @return the autonomy of the now new top car
 */
int calcolaNuovaAutonomia(stazione* staz, int oldMax){
    int i;
    int newMax = 0;
    for(i=0; i<staz->indice; i++){
        if(staz->parcheggio[i] > newMax){
            newMax = staz->parcheggio[i];
            if(newMax == oldMax){//se scopriamo che c'era un altra auto uguale a quella piu potente: finiamo subito
                return oldMax;
            }
        }
    }
    return newMax;
}

/**
 * this function is called when a top car is removed from a station. It does not affect the station itself:
 *  we just verify if now there are some station no more reachable that still think we are their max Next or Pre Station.
 *  in this case we proceed to find and update maxNextStazione and maxPreStazione for all those stations.
 * @param staz station where we removed the top cra
 * @param oldAutonomia autonomy of the old removed top car
 * @return 1, 0 if something went wrong
 */
int removedMaxAuto(stazione * staz, int oldAutonomia){
    //conf iniziale
    int x, y, z;
    stazione * miaStaz = NULL;
    stazione * precedenti = NULL;
    stazione * successive = NULL;
    miaStaz = staz;
    z = staz->km;
    x = z + oldAutonomia;
    y = z - oldAutonomia;
    if(x>999999999) x=999999999;
    if(y<0) y=0;
    successive = ricercaVicinoDS(x);
    precedenti = ricercaVicinoDS(y);

    //if(precedenti == NULL || successive == NULL) return 2;// (if no station is available how can we remove a car??)

    //ricerca vicino ritorna, dato un km una stazione vicina. ma non sappiamo a priori se e maggiore o minore del kilometro.
    if(successive->km > x) successive = successive->preStazione;
    if(precedenti->km < y) precedenti = precedenti->nextStazione;
    //fine conf iniziale
    while(successive != miaStaz){
        //if(successive->maxPreStazione != NULL){//usless
        if(successive->maxPreStazione->km < z) break;//se la stazione (dopo di me) che riuscivo a ragiungere con la mia top car è raggiunta da
        //una stazione precedente a me, ho finito: la mia rimozione non induce alcuna modifica
        //}
        if((successive->km - miaStaz->km) > miaStaz->autoMax){
            aggiornaMaxPrevStaz(miaStaz, successive);
        }else{//le prossime stazioni le raggiungevo e le raggiungo ancora
            break;
        }
        successive = successive->preStazione;
    }
    while(precedenti != miaStaz){
        //if(precedenti->maxNextStazione != NULL){//ulsess
        if(precedenti->maxNextStazione->km > z) break;//se la stazione (prima di me) che riuscivo a ragiungere con la mia top car è raggiunta da
        //una stazione successiva a me, ho finito: la mia rimozione non induce alcuna modifica
        //}
        if((miaStaz->km - precedenti->km) > miaStaz->autoMax){
            aggiornaMaxNextStaz(miaStaz, precedenti);
        }else{//le prossime stazioni le raggingevo e le raggiungo ancora
            break;
        }
        precedenti = precedenti->nextStazione;
    }
    return 1;
}

/**
 * NB never pass @partenza=@arrivo
 * @param partenza station where we removed the top car or any station NEXT it but before @arrivo
 * @param arrivo station (bigger than partenza) that we were able to reach with our top car but now not anymore. And we were
 * the maxPreviusStation for @arrivo
 * @return 1 if we found a new maxPrevStaz for @arrivo, 0 otherwise
 */
int aggiornaMaxPrevStaz(stazione* partenza, stazione* arrivo){//direzione: AVANTI
    //arrivo sono tutte le stazioni a cui prima arrivavo dalla
    // mia stazione con l auto max ed ora tecnicamente non piu. partenza è o la mia stazione dalla quale ho rimosso l auto
    //oppure è una stazione a lei succssiva (ovviamente precedente ad arrivo)
    if((arrivo->km - partenza->km) <= partenza->autoMax){//arrivo raggiungibile da partenza: ok
        arrivo->maxPreStazione = partenza;
        return 1;
    }else{//arrivo non raggiugibile da partenza: mi sposto alla next stazione di partenza. continuo cosi ricorsivamente
        //finche o non raggiungo una stazione in grado di arrivare ad arrivo, oppure raggiungo arrivo stesso.
        //in questo ultimo caso, nessuna stazione riesce a raggiungere l arrivo procedendo in avanti
        //if(partenza->nextStazione != NULL){//usless control?
        if(partenza->nextStazione == arrivo){
            arrivo->maxPreStazione = NULL;
            return 0;
        }
        return aggiornaMaxPrevStaz(partenza->nextStazione, arrivo);
        //}
        return 0;//technically, we should never reach this point: it means partenza->nextStazione == NULL and we haven't
        // incontered arrivo yet. since it's an ordered list and arrivo comes after partenza (in this case) this can't happen.
    }
}

/**
 * NB never pass @partenza=@arrivo
 * @param partenza station where we removed the top car or any station PREVIOUS it but after @arrivo
 * @param arrivo station (lower than partenza) that we were able to reach with our top car but now not anymore. And we were
 * the maxNextStation for @arrivo
 * @return 1 if we found a new maxNextStaz for @arrivo, 0 otherwise
 */
int aggiornaMaxNextStaz(stazione* partenza, stazione* arrivo){//direzione INDIETRO
    //arrivo sono tutte le stazioni a cui prima arrivavo dalla
    // mia stazione con l auto max ed ora tecnicamente non piu. partenza è o la mia stazione dalla quale ho rimosso l auto
    //oppure è una stazione a lei precedente (ovviamente successiva ad arrivo)
    if((partenza->km - arrivo->km) <= partenza->autoMax){//arrivo raggiungibile da partenza: ok
        arrivo->maxNextStazione = partenza;
        return 1;
    }else{//arrivo non raggiugibile da partenza: mi sposto alla prev stazione di partenza. continuo cosi ricorsivamente
        //finche o non raggiungo una stazione in grado di arrivare ad arrivo, oppure raggiungo arrivo stesso.
        //in questo ultimo caso, nessuna stazione riesce a raggiungere l arrivo procedendo in indietro.
        //if(partenza->preStazione != NULL){
        if(partenza->preStazione == arrivo){
            arrivo->maxNextStazione = NULL;
            return 0;
        }
        return aggiornaMaxNextStaz(partenza->preStazione, arrivo);
        //}
        return 0;//technically, we should never reach this point: it means partenza->preStazione == NULL and we havent
        // incontered arrivo yet. since its a ordered list and arrivo comes before partenza (in this case) this cant happen.
    }
}

// end of rimuoviAuto Utils

/**
 * removes station at given kilomter ( if present)
 * @param kilometro of the station to be removed
 * @return 1 if the station is correctly removed, 0 otherwise
 */
int rimuoviStazione(int kilometro){
    stazione* s = NULL;
    int oldAuto = 0;
    if(sIniziale == NULL){
        //printf("nessuna stazione esistente, rimozione della stazione %d impossibile\n", kilometro);
        printf("non demolita\n");
        return 0;
    }else{
        s = dammiStazione(kilometro);
        if(s == NULL){
            //printf("stazione %d non esistente. rimozione impossbile\n", kilometro);
            printf("non demolita\n");
            return 0;
        }else{
            oldAuto = s->autoMax;
            s->autoMax = 0;
            removedMaxAuto(s, oldAuto);
            eliminaDS(s);
            rimuoviNodoAshTable(kilometro);
            rimuoviNodoLista(s);
            //printf("stazione al kilometro %d rimossa\n", kilometro);
            printf("demolita\n");
            return 1;
        }
    }
    return 0; //technically, we shuold not reach this point
}

/**
 * this function removes a station from daniel'sTree: sets to null pointer to station into leaf node and sets validity bit to 0.
 * calls riparaEliminazione() to clean
 * @param s station to remove
 * @return 1
 */
int eliminaDS(stazione* s){
    s->puntDS->c[s->km%10].nxt = NULL;
    s->puntDS->c[s->km%10].x = 0;
    s->puntDS->cont = (char)((int)s->puntDS->cont - 1);
    return riparaEliminazione(s->puntDS, s->km/10, nCifre(s->km));
}

/**
 * this function checks if current (given) daniel'sTree's node has any child (we simpy look at cont).
 * Only if current node has no children (cont = 0):
 *    we decrement our father's cont and remove the reference that our father has to us.
 *    Then we just call ourself recursively on the father.
 * @param v current daniel'sTree's node
 * @param k portion of the kilometer of the station we removed ( each cicle the last significant digit will be processed and removed)
 * @param cifre current digit of the kilometer
 * @return 1
 */
int riparaEliminazione(vettoreCifre *v, int k, int cifre) {
    if (v->cont == 0 && v->padre != NULL) {
        if (v->padre == &inizio) {
            v->padre->c[cifre].nxt = NULL;
            v->padre->c[cifre].x = 0;
            v->padre->cont = (char) (int) (v->padre->cont - 1);
        } else {
            v->padre->c[k % 10].nxt = NULL;
            v->padre->c[k % 10].x = 0;
            v->padre->cont = (char) (int) (v->padre->cont - 1);
        }
        return riparaEliminazione(v->padre, k / 10, cifre);
    }
    return 1;
}

/**
 * removes station at given kilometer from HashTable, if present
 * @param kilometro
 * @return 1 if removed, 0 otherwise
 */
int rimuoviNodoAshTable(int kilometro) {
    nodoHashTable *y = NULL;
    nodoHashTable *prev = NULL;
    y = &table[ash(kilometro)];
    prev = y;
    while (y->sameAsh != NULL) {//we go here if we have other buckets attacched: can be 1 entry or nested
        if (y->miaStazione->km == kilometro) {
            if (prev != y) {//its not on the first entry
                prev->sameAsh = y->sameAsh;
                y->miaStazione = NULL;//usless
            } else {//its on the first entry and has other buckets attached
                y->miaStazione = y->sameAsh->miaStazione;
                y->sameAsh = y->sameAsh->sameAsh;
            }
            return 1;
        } else {//
            prev = y;
            y = y->sameAsh;
        }
    }
    if (y->miaStazione != NULL) {//we go here only if the station is in the last bucket or the fist entry.
        //in both cases it hasnt a next (same ash) node attached.
        if (y->miaStazione->km == kilometro) {
            y->miaStazione = NULL;//usless
            if (prev != y) {//its not on the first entry
                prev->sameAsh = NULL;
            }
            return 1;
        }
    }
    return 0; //station not found and not removed. (y->miaStazione == NULL or y->miaStazione->km != kilometro)
}

/**
 * this function removes given station from linked list of stations
 * @param staz pointer to station
 * @return 1
 */
int rimuoviNodoLista(stazione* staz){
    stazione * s = NULL;
    s = staz;
    if(sIniziale == s){//first element of the list
        if(s->nextStazione == NULL){//unic element of the list (first AND last)
            sIniziale = NULL;
            //initializeStation(s);
            return 1;
        }else{//first element of the list but not the last
            sIniziale = s->nextStazione;
            sIniziale->preStazione = NULL;
            //initializeStation(s);
            return 1;
        }
    }else{//not the first element of the list
        if(s->nextStazione == NULL){//last element of the list
            s->preStazione->nextStazione = NULL;
            //initializeStation(s);
            return 1;
        }else{//in the middle of the list
            s->preStazione->nextStazione = s->nextStazione;//adjusting previous station's pointer
            s->nextStazione->preStazione = s->preStazione;//adjusting next station's pointer
            //initializeStation(s);
            return 1;
        }
    }
    return 0; //technically, we should never reach this point
}

//end of rimuoviStazione Utils

/**
 * calls pianificaAvanti() if arrivo>partenza; pianificaIndietro() otherwise
 * @param partenza km of starting station
 * @param arrivo km of end station
 * @return 1 if we found a trip, 0 otherwise
 */
int pianificaPercorso(int partenza, int arrivo){
    stazione* p = NULL;
    stazione* a = NULL;
    p = dammiStazione(partenza);
    if(p == NULL){
        //printf("errore: stazione/i inesistenti\n");
        printf("nessun percorso\n");
        return 0;
    }
    if(partenza == arrivo){//partenza e arrivo coincidono
        printf("%d\n", partenza);
        return 1;
    }
    a = dammiStazione(arrivo);
    if(a == NULL){//controllo inutule, le stazioni date esistono per forza
        //printf("errore: stazione/i inesistenti\n");
        printf("nessun percorso\n");
        return 0;
    }else{
        initializeTappe();
        if(partenza < arrivo){
            if(pianificaAvanti(p, a, a->km) !=-2 ){
                stampaTappe();
            }
        }else{
            if(pianificaIndietro(p, a, a->km) !=-2){
                trovaAlternative();
                trovaAlternativeTest();
                trovaAlternativeInfame();
                stampaTappe();
            }
        }
        return 1;
    }
    return 0;//technically, we should never reach this point
}

/**
 * NB @partenza->km < @arrivo->km
 * this functions computes the trip by looking at the journey backwards: starting from the arrival we loop back using
 * maxPrevStaz reference until we reach the start.
 * @param partenza pointer to start station
 * @param arrivo pointer to intermediate station
 * @param veroArrivo km of end station
 * @return 1 if we found a trip, -2 otherwise
 */
int pianificaAvanti(stazione *partenza, stazione *arrivo, int veroArrivo) {
    if ((arrivo->km - partenza->km) <= partenza->autoMax) {//partenza arriva ad arrivo
        //printf("%d", partenza->km);
        //printf(" %d", arrivo->km);
        inserisciTappa(partenza->km);//aggiungo sia partenza che arrivo alle tappe
        inserisciTappa(arrivo->km);
    } else {
        if (arrivo->maxPreStazione == NULL) {
            printf("nessun percorso\n");
            return -2;
        }
        if (arrivo->maxPreStazione->km > partenza->km) {//ce una stazione intermedia
            if(pianificaAvanti(partenza, arrivo->maxPreStazione, veroArrivo) == -2){
                return -2;
            }
            inserisciTappa(arrivo->km);//messo dopo pianifica avanti per cercare di mantenere ordinata la lista tappe
            //printf(" %d", arrivo->km);
        } else {//esiste una stazione precedente a partenza che arriva ad arrivo: a noi non va bene
            return piccoloPianificaAvanti(partenza, arrivo, partenza->km);
        }
    }
    return 1;
}

/**
 * this functions stores given km into tappe array. once it will be complete, we can print it.
 * @param k kilometer of station we stop
 * @return 1, 0 if something went wrong
 */
int inserisciTappa(int k){
    if(indiceTappe<dimVettoreTappe){
        tappe[indiceTappe] = k;
        indiceTappe++;
    }else{
        printf("ERRORE, spazio tappe esaurito\n");
        return 0;
    }
    return 1;
}

/**
 * this function computes the trip. it works in the same direction of march
 * @param partenza intermediate station
 * @param arrivo end station
 * @param veroInizio km of start station
 * @return -2 if we can't find any trip. An integer between 0 and 999.999.999 otherwise
 */
int piccoloPianificaAvanti(stazione *partenza, stazione *arrivo, int veroInizio) {
    int newArrivo = 0;
    if (partenza == arrivo) {
        printf("nessun percorso\n");
        return -2;
    } else {
        if (partenza->autoMax >= (arrivo->km - partenza->km)) {//da partenza arrivo ad arrivo
            inserisciTappa(arrivo->km);
            if(partenza->km == veroInizio){
                inserisciTappa(partenza->km);
            }
            return partenza->km;
        } else {//da partenza non arrivo ad arrivo: uso la ricorsione:
            newArrivo = piccoloPianificaAvanti(partenza->nextStazione, arrivo,veroInizio);//chiedo di trovare un'alternativa al successivo di partenza
            if(partenza->km == veroInizio){//can be optimized
                return piccoloPianificaAvanti(partenza, dammiStazione(newArrivo), veroInizio);//dopodiche mi preoccupo di riuscire ad arrivare dalla vera
                //partenza alla tappa intermedia
            }else{
                return newArrivo;
            }
        }
    }
}

//end of pianificaAvanti Utils

/**
 * prints tappe array on StandardOutput
 */
void stampaTappe(){
    int i;
    for(i=0;i<indiceTappe-1;i++){
        printf("%d ", tappe[i]);
    }
    printf("%d\n", tappe[indiceTappe-1]);
}

/**
 * NB @partenza->km > @arrivo->km
 * this functions computes the trip by looking at the journey backwards: starting from the arrival we loop back using
 * maxNextStaz reference until we reach the start. Its works similar to pianificaAvanti()
 * @param partenza pointer to start station
 * @param arrivo pointer to intermediate station
 * @param veroArrivo km of end station
 * @return 1 if we found a trip, -2 otherwise
 */
int pianificaIndietro(stazione* partenza, stazione* arrivo, int veroArrivo){
    if((partenza->km - arrivo->km) <= partenza->autoMax){
        inserisciTappa(partenza->km);
        inserisciTappa(arrivo->km);
    }else{
        if(arrivo->maxNextStazione == NULL){
            printf("nessun percorso\n");
            return -2;
        }
        if(arrivo->maxNextStazione->km < partenza->km){//c'è una stazione intermedia
            if(pianificaIndietro(partenza, arrivo->maxNextStazione, veroArrivo) == -2){
                return -2;
            }
            inserisciTappa(arrivo->km);
        }else{
            return piccoloPianificaIndietro(partenza, arrivo, partenza->km);
        }
    }
    return 1;
}

/**
 * this function computes the trip. it works in the same direction of march
 * @param partenza intermediate station
 * @param arrivo end station
 * @param veroInizio km of start station
 * @return -2 if we can't find any trip. An integer between 0 and 999.999.999 otherwise
 */
int piccoloPianificaIndietro(stazione* partenza, stazione* arrivo, int veroInizio){
    int newArrivo = 0;
    if(partenza == arrivo){
        printf("nessun percorso\n");
        return -2;
    }else{
        if(partenza->autoMax >= (partenza->km - arrivo->km)){
            inserisciTappa(arrivo->km);
            if(partenza->km == veroInizio){
                inserisciTappa(partenza->km);
            }
            return partenza->km;
        }else{
            newArrivo = piccoloPianificaIndietro(partenza->preStazione, arrivo, veroInizio);
            if(partenza->km == veroInizio){
                return piccoloPianificaIndietro(partenza, dammiStazione(newArrivo), veroInizio);
            }else{
                return newArrivo;
            }
        }
    }
}

// end of pianificaIndietro Utils

/**
* NOTE: this mechanism will generate correct result if there are no more than 1 possible different trip with minimum
 * number of steps from 2 points. If we have more than 1 possible trip, we need to choose the one that starts with
 * stations near km 0. If @partenza < @arrivo my algorithm works perfectly, generating the right steps.
 * Unfortunately, if @partenza > @arrivo, my algorithm will output the alternative journeys. That's why the necessity
 * of implementing the underlying functions:
*/
int trovaAlternative(){
    int i;
    int autonomia = 0;
    int maxDepth = 0; //lower it is, more effective
    int myDepth = 0;
    stazione* s = NULL;
    stazione* succ = NULL;
    for(i=0; i<indiceTappe-2; i++){//faccio passare le tappe
        s = dammiStazione(tappe[i]);//prendo una stazione
        succ = dammiStazione(tappe[i+1])->preStazione;//prendo la stazione precedente a quella alla quale dovrei
        //arrivare a partire da s
        autonomia = s->autoMax;
        maxDepth = maxPrevStaz(s);
        while((s->km - succ->km) <= autonomia){//per tutte le stazioni raggiungibili da s (successive a succ)
            if(succ->autoMax >= (succ->km - dammiStazione(tappe[i+2])->km)){//se anche loro mi permettono di arrivare
                //almeno fin dove arrivava succ, le scelgo (sono piu vicine al inizio del autostrada)
                if(i == (indiceTappe-3)){//ultimo step: non ci interessa quella che va piu lontano ma ci basta che arivi a destinazione
                    swapTappe(i+1, succ->km);
                }else{
                    myDepth = maxPrevStaz(succ);
                    if(myDepth < 0){
                        //swapTappe(i+1, succ->km);
                        printf("errore, my depth è minore di zero\n");
                    }else{
                        if(myDepth <= maxDepth){
                            maxDepth = myDepth;
                            swapTappe(i+1, succ->km);
                        }
                    }
                }
            }
            succ = succ->preStazione;
        }
    }
    return 1;
}
int maxPrevStaz(stazione * s){
    int posizione = s->km-s->autoMax;
    stazione * previus = NULL;
    previus = ricercaVicinoDS(posizione);
    if(previus == NULL) return -1;
    if(previus == s) return -1;
    if(previus->km == posizione){
        return previus->km;
    }else{
        if(previus->km > posizione){
            if(previus->km < s->km){
                return previus->km;
            }else{
                return -1;
            }
        }else{//previus è precedente a posizione(km a cui arrivo al massimo al indietro)
            if(previus->nextStazione != NULL){
                if(previus->nextStazione != s){
                    return previus->nextStazione->km;
                }
            }
        }
    }
    return -1;
}
/**
 * place @km value into tappe array at @index position
 * @param indice index of tappe
 * @param km value
 */
void swapTappe(int indice, int km){
    tappe[indice] = km;
}
int trovaAlternativeTest(){
    int i;
    int autonomia = 0;
    int cambiato = 0;
    stazione* s = NULL;
    stazione* succ = NULL;
    for(i=0; i<indiceTappe-2; i++){//faccio passare le tappe
        s = dammiStazione(tappe[i]);//prendo una stazione
        succ = dammiStazione(tappe[i+1])->preStazione;//prendo la stazione precedente a quella alla quale dovrei
        //arrivare a partire da s
        autonomia = s->autoMax;
        while((s->km - succ->km) <= autonomia){//per tutte le stazioni raggiungibili da s (successive a succ)
            if(succ->autoMax >= (succ->km - dammiStazione(tappe[i+2])->km)){//se anche loro mi permettono di arrivare
                //almeno fin dove arrivava succ, le scelgo (sono piu vicine al inizio del autostrada)
                //ultimo step: non ci interessa quella che va piu lontano ma ci basta che arivi a destinazione
                swapTappe(i+1, succ->km);
                cambiato = 1;
            }
            succ = succ->preStazione;
        }
    }
    return cambiato;
}
int trovaAlternativeInfame(){
    int i;
    stazione* s = NULL;
    stazione* trovata = NULL;
    stazione* prec = NULL;
    stazione* doppioPrec = NULL;
    stazione* sInfame = NULL;
    for(i=indiceTappe-1; i>1; i--){
        s = dammiStazione(tappe[i]);
        prec = dammiStazione(tappe[i-1]);
        doppioPrec = dammiStazione(tappe[i-2]);
        trovata = s;
        while(trovata->km < prec->km){//ne cerco una piu vicina
            if(trovata->autoMax >= (trovata->km - s->km)){//trovata mi permette di arrivare a s
                if(doppioPrec->autoMax >= (doppioPrec->km - trovata->km)){//o ci arrivo dalla stazione precedente
                    swapTappe(i-1, trovata->km);
                    return trovaAlternativeInfame();
                }else{//oppure provo ad arrivarci da una precedente ad essa ma che venga prima di quella precedente ancora
                    if(i>2){//fatto sse i>2, all ultimo passo non va verificato
                        sInfame = NULL;
                        sInfame = trovaStazioneInfame(trovata, doppioPrec, dammiStazione(tappe[i-3]));
                        if(sInfame != NULL){
                            swapTappe(i-1, trovata->km);
                            swapTappe(i-2, sInfame->km);
                            return trovaAlternativeInfame();
                        }
                    }
                }
            }
            trovata = trovata->nextStazione;
        }
    }
    return 1;
}
stazione* trovaStazioneInfame(stazione* trov, stazione* min, stazione* max){
    while(min!=max){
        if(min->autoMax >= (min->km - trov->km)){//se da min arrivo ad trov
            return min;//return min
        }
        min = min->nextStazione;
    }
    return NULL;//stazione non esistente
}

// end of pianificaPercorso() Utils

// 02-08-2023

//EOF