#include <stdio.h>
#include <stdlib.h>

#define dimAshTable 100000
#define dimVettoreTappe 1000
#define dimMallocVettori 1000

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

typedef struct nodoAshTable {
    stazione *miaStazione;
    struct nodoAshTable *sameAsh;
} nodoAshTable;

typedef struct cifra{
    unsigned x:1;
    void * nxt;
} cifra;

typedef struct vettoreCifre{
    cifra c[10];
    struct vettoreCifre * padre;
    char cont;
} vettoreCifre;

void stampaStazioni();

void verificaStazioni();
void initializeBucket(nodoAshTable* n);
int ash(int km);
void initializeStation(stazione* s);
int insertIntoTable(int kilometro, stazione* newStaz);
int cercaInTab(int kilometro);
int aggiornaDipendenze(stazione* staz);
stazione * dammiStazione(int kilometro);
int addedMaxAuto(stazione * staz, int autonomia);
int aggiornaMaxPrevStaz(stazione* partenza, stazione* arrivo);
int aggiornaMaxNextStaz(stazione* partenza, stazione* arrivo);
int removedMaxAuto(stazione* staz, int oldAutonomia);
int calcolaNuovaAutonomia(stazione* staz, int oldMax);
int rimuoviNodoLista(stazione* staz);
int rimuoviNodoAshTable(int kilometro);
int pianificaAvanti(stazione* partenza, stazione* arrivo, int veroArrivo);
int pianificaIndietro(stazione* partenza, stazione* arrivo, int veroArrivo);
int piccoloPianificaAvanti(stazione* partenza, stazione* arrivo, int veroInizio);
void initializeTappe();
int inserisciTappa(int k);
void stampaTappe();
void swapTappe(int indice, int km);
int piccoloPianificaIndietro(stazione* partenza, stazione* arrivo, int veroInizio);
int trovaAlternative();
int trovaAlternative2();
int maxPrevStaz(stazione* s);
int trovaAlternativeTest();
int trovaAlternativeInfame();
int aggiornaMaxNext(stazione* arrivo, stazione* partenza);
int aggiornaMaxPrev(stazione* arrivo, stazione* partenza);

int ricercaDS(vettoreCifre * v, int cifra, int step, stazione* s);
int creaDS(vettoreCifre * v, int cifra, int step, stazione* s);
int riparaEliminazione(vettoreCifre * v, int k, int cifre);
stazione * ricercaVicinoDSVera(int kilometro, vettoreCifre * v, int cifra, int step);
stazione* ricercaMaxDS(vettoreCifre * v, int c);
stazione* ricercaMinDS(vettoreCifre * v, int c);

//variabili globali
nodoAshTable table[dimAshTable];
int tappe[dimVettoreTappe];
int indiceTappe = 0;

stazione *sIniziale = NULL;

//malloc support for Nodes
int contatore = 0;
stazione *inizioMemNodi = NULL;
stazione *fineMemNodi = NULL;
stazione *sonoAllaFine = NULL;

//malloc support for Busckets
int contatore2 = 0;
nodoAshTable *inizioMemBuckets = NULL;
nodoAshTable *fineMemBuckets = NULL;
nodoAshTable *sonoAllaFine2 = NULL;

//support for DS
char stringa[10];
vettoreCifre inizio;
int contatore3 = 0;
vettoreCifre * inizioMemVettori = NULL;
vettoreCifre * fineMemVettori = NULL;
vettoreCifre * sonoAllaFine3 = NULL;
//fine variabili globali

//inizio DS
int inizializzaVettore(vettoreCifre* v){
    int i = 0;
    for(i=0; i<10; i++){
        v->c[i].x = 0;
        v->c[i].nxt = NULL;
        v->padre = NULL;
        v->cont = (char)0;
    }
    return 1;
}

int inizializzaStringa(){
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
int aggiornaStringa(int km, int ncifre){
    int i = 0;
    for(i=0; i<ncifre; i++){
        stringa[i] = km%10;
        km = km/10;
    }
    return 1;
}

int nCifre(int n){
    int i = 1;
    while(n >= 10){
        n = n / 10;
        i++;
    }
    return i;
}

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

int ultimoStepAggStaz(vettoreCifre * v, int cifra, stazione* s){
    v->c[cifra].nxt = (stazione *) s;
    v->c[cifra].x = 1;
    v->cont = (char)((int)v->cont + 1);
    s->puntDS = v;
    return 1;
}

int aggiungiStazioneDS(int kilometro, stazione* s){
    int km = kilometro;
    int c = 0;
    inizializzaStringa();
    c = nCifre(km);
    aggiornaStringa(km, c);
    return ricercaDS(&inizio, c, c, s);
}

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

int eliminaDS(stazione* s){
    s->puntDS->c[s->km%10].nxt = NULL;
    s->puntDS->c[s->km%10].x = 0;
    s->puntDS->cont = (char)((int)s->puntDS->cont - 1);
    return riparaEliminazione(s->puntDS, s->km/10, nCifre(s->km));
}

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

//NB non chiamare la funzione se la struttura dati è vuota o incosistente, altrimenti non funziona!
//NB non chiamare la funzione per cercare una stazione esistente! passa il kilometro a cui non esistono stazioni!
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
//fine DS

stazione *dammiMemoriaNodi() {
    if (inizioMemNodi == NULL) {//primo nodo in assoluto
        //printf("creo spazio di memoria\n");
        if ((inizioMemNodi = (stazione *) malloc(64000 * sizeof(stazione))) != NULL) {
            fineMemNodi = inizioMemNodi + 64000 * sizeof(stazione);
            sonoAllaFine = inizioMemNodi;
            contatore = 0;
            return &inizioMemNodi[contatore];
        } else { printf("errore creazione spazio memoria aggiuntivo peer nodi\n"); }
    } else {
        sonoAllaFine = sonoAllaFine + sizeof(stazione);
        if (sonoAllaFine >= fineMemNodi) {//ho finito la memoria fornita
            //printf("ho finito la memoria fornita\n");
            if ((inizioMemNodi = (stazione *) malloc(64000 * sizeof(stazione))) != NULL) {
                fineMemNodi = inizioMemNodi + 64000 * sizeof(stazione);
                sonoAllaFine = inizioMemNodi;
                contatore = 0;
                return &inizioMemNodi[contatore];
            } else { printf("errore creazione spazio memoria aggiuntivo\n"); }
        }
    }
    contatore = contatore + 1;
    return &inizioMemNodi[contatore];
}//ok

nodoAshTable *dammiMemoriaBuckets() {
    if (inizioMemBuckets == NULL) {//primo nodo in assoluto
        //printf("creo spazio di memoria\n");
        if ((inizioMemBuckets = (nodoAshTable *) malloc(64000 * sizeof(nodoAshTable))) != NULL) {
            fineMemBuckets = inizioMemBuckets + 64000 * sizeof(nodoAshTable);
            sonoAllaFine2 = inizioMemBuckets;
            contatore2 = 0;
            return &inizioMemBuckets[contatore2];
        } else { printf("errore creazione spazio memoria aggiuntivo peer nodi\n"); }
    } else {
        sonoAllaFine2 = sonoAllaFine2 + sizeof(nodoAshTable);
        if (sonoAllaFine2 >= fineMemBuckets) {//ho finito la memoria fornita
            //printf("ho finito la memoria fornita\n");
            if ((inizioMemBuckets = (nodoAshTable *) malloc(64000 * sizeof(nodoAshTable))) != NULL) {
                fineMemBuckets = inizioMemBuckets + 64000 * sizeof(nodoAshTable);
                sonoAllaFine2 = inizioMemBuckets;
                contatore2 = 0;
                return &inizioMemBuckets[contatore2];
            } else { printf("errore creazione spazio memoria aggiuntivo\n"); }
        }
    }
    contatore2 = contatore2 + 1;
    return &inizioMemBuckets[contatore2];
}//ok

int aggiungiStazione2(int staz) {
    printf("aggiungiStazione km %d\n", staz);
    return 1;
}

int aggiungiAuto2(int staz, int supp) {
    printf("aggiungiAuto autonomia %d km %d\n", supp, staz);
    return 1;
}

int rimuoviAuto2(int staz, int supp) {
    printf("rimuoviAuto autonomia %d km %d\n", supp, staz);
    return 1;
}

int rimuoviStazione2(int staz) {
    printf("rimuovoStazione km %d\n", staz);
    return 1;
}

void pianificaPercorso2(int inizio, int fine) {
    printf("pianifico percorso tra %d e %d\n", inizio, fine);
}

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
int piccoloPianificaAvanti(stazione *partenza, stazione *arrivo, int veroInizio) {
    int newArrivo = 0;
    if (partenza == arrivo) {
        printf("nessun percorso\n");
        return -2;
    } else {
        if (partenza->autoMax >= (arrivo->km - partenza->km)) {
            inserisciTappa(arrivo->km);
            if(partenza->km == veroInizio){
                inserisciTappa(partenza->km);
            }
            return partenza->km;
        } else {
            newArrivo = piccoloPianificaAvanti(partenza->nextStazione, arrivo,veroInizio);
            if(partenza->km == veroInizio){//can be optimized
                return piccoloPianificaAvanti(partenza, dammiStazione(newArrivo), veroInizio);
            }else{
                return newArrivo;
            }
        }
    }
}

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


int trovaAlternative2(){
    int i;
    int x;
    stazione* s = NULL;
    stazione* succ = NULL;
    stazione* duobleSucc = NULL;
    for(i=indiceTappe-1; i>1; i--){
        s = dammiStazione(tappe[i]);
        x = dammiStazione(tappe[i-1])->km;
        succ = s->nextStazione;
        while(succ->km < x){//tra me la prossima stazione (percorrendo in avanti per l autostrada,ossia dall arrivo alla partenza)
            if(succ->autoMax >= (succ->km - s->km)){//se tale stazione arriva a me
                duobleSucc = dammiStazione(tappe[i-2]);
                if(duobleSucc->autoMax >= (duobleSucc->km - succ->km)){//se la successiva della prissima mi permette di arrivare anche a me(succ), allora la prendo
                    swapTappe(i-1, succ->km);
                }
            }
            succ = succ->nextStazione;
        }
    }
    return 1;
    trovaAlternative2();
}
int trovaAlternative3(){
    int i;
    int autonomia = 0;
    stazione* s;
    stazione* succ;
    for(i=0; i<indiceTappe-2; i++){//faccio passare le tappe
        s = dammiStazione(tappe[i]);//prendo una stazione
        succ = dammiStazione(tappe[i+1])->preStazione;//prendo la stazione precedente a quella alla quale dovrei
        //arrivare a partire da s
        autonomia = s->autoMax;
        while((s->km - succ->km) <= autonomia){//per tutte le stazioni raggiungibili da s (successive a succ)
            if(succ->autoMax >= (succ->km - dammiStazione(tappe[i+2])->km)){//se anche loro mi permettono di arrivare
                //almeno fin dove arrivava succ, le scelgo (sono piu vicine al inizio del autostrada)
                swapTappe(i+1, succ->km);
            }
            succ = succ->preStazione;
        }
    }
    return 1;
}


int trovaAlternative(){//todo modifica usando massima stazione raggiunta anzichè il kiolometro
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
int trovaAlternativeTest(){//todo modifica usando massima stazione raggiunta anzichè il kiolometro
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
stazione* trovaStazioneInfame(stazione* trov, stazione* min, stazione* max){
    while(min!=max){
        if(min->autoMax >= (min->km - trov->km)){//se da min arrivo ad trov
            return min;//return min
        }
        min = min->nextStazione;
    }
    return NULL;//stazione non esistente
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

int maxPrevStaz(stazione* s){//returns max previus reachable station or -1 if there is no station reachable
    int max = -1;
    stazione* prec = s->preStazione;
    while(prec != NULL){
        if((s->km - prec->km) <= s->autoMax){
            max = prec->km;
            prec = prec->preStazione;
        }else{
            break;
        }
    }
    return max;
}

int rimuoviAuto(int kmStazione, int autonomia){
    int i, j;
    stazione* s = NULL;
    s = dammiStazione(kmStazione);
    if(s == NULL){
        //printf("impossibile rimuovere auto con autonomia %d , stazione al kilometro %d inesistente\n", autonomia, kmStazione);
        printf("non rottamata\n");
        //return 0; //return here if station is not present
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
        //return 0; //return here if auto is not present
    }
    return 0;//return here if either there is no station or no car
}
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

int aggiungiAuto(int kmStazione, int autonomia, int stampa){//stampa == 1 only if the function is called from a aggiungi-auto request.
    //stampa == 0 if the function is calleed from a aggiungi-stazione request
    stazione* s = NULL;
    s = dammiStazione(kmStazione);
    if(s == NULL){
        //printf("impossibile aggiungere automobile con autonomia %d: la stazione al kilometro %d non esiste\n", autonomia, kmStazione);
        if(stampa) printf("non aggiunta\n");//CONTINUA DA QUA
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

int aggiungiStazione(int kilometro) {
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
            if (s->nextStazione == NULL) {//inserimento all'ultimo posto
                aggiungiStazioneDS(kilometro, support);
                s->nextStazione = (struct stazione *) support;
                support->preStazione = (struct stazione *) s;
                insertIntoTable(kilometro, support);
                aggiornaDipendenze(support);
                return 1;
            }
            return 0;//technically, we should NEVER reach this point. it means we exited while(true) loop (wrong behaviour)
        }
    }
}

int rimuoviNodoAshTable(int kilometro) {
    nodoAshTable *y = NULL;
    nodoAshTable *prev = NULL;
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

int insertIntoTable(int kilometro, stazione* newStaz){
    nodoAshTable * puntNodoTab = NULL;
    puntNodoTab = &table[ash(kilometro)];
    if(puntNodoTab->miaStazione == NULL){//inserimento diretto nella table
        puntNodoTab->miaStazione = newStaz;
        return 1;
    }else{
        while(puntNodoTab->sameAsh != NULL){
            puntNodoTab = (nodoAshTable *)puntNodoTab->sameAsh;
        }
        puntNodoTab->sameAsh = (struct nodoAshTable *)dammiMemoriaBuckets();
        initializeBucket((nodoAshTable *)puntNodoTab->sameAsh);
        ((nodoAshTable *)(puntNodoTab->sameAsh))->miaStazione = newStaz;
        return 1;
    }
    return 0;//technically, we should never reach this point
}
int cercaInTab(int kilometro){
    nodoAshTable * y = NULL;
    y = &table[ash(kilometro)];
    while(y->sameAsh != NULL){
        if(y->miaStazione->km == kilometro){//trovata
            //printf("stazione al kilometro %d trovata\n", kilometro);
            return 1;
        }
        y = (nodoAshTable *) y->sameAsh;
    }
    if(y->miaStazione != NULL){
        if(y->miaStazione->km == kilometro){
            //printf("stazione al kilometro %d trovata\n", kilometro);
            return 1;
        }
    }
    //non trovata
    return 0;
}
stazione * dammiStazione(int kilometro){
    nodoAshTable* y = NULL;
    y = &table[ash(kilometro)];
    while(y->sameAsh != NULL){
        if(y->miaStazione->km == kilometro){
            return y->miaStazione;
        }
        y = (nodoAshTable *) y->sameAsh;
    }
    if(y->miaStazione != NULL){
        if(y->miaStazione->km == kilometro){
            return y->miaStazione;
        }
    }
    return NULL;
}



int ash(int km) {
    unsigned long n; //for stations above half million kilometers, using basic integer will result in overflow and wrong results
    n = (unsigned long) km;
    n = (n * (n + 3)) % dimAshTable; //using Knuth's ash function
    km = (int) n;
    return km;
}//ok
int addedMaxAuto2(stazione * staz, int autonomia){//this function can be void
    stazione* miaStaz = NULL;
    stazione* precedenti = NULL;
    stazione* successive = NULL;
    //printf("you just added a top car\n");
    miaStaz = staz;
    successive = staz;
    precedenti = staz;
    //aggiornamento delle stazioni successive
    while (successive->nextStazione != NULL){
        successive = successive->nextStazione;
        if((successive->km - miaStaz->km) > autonomia){//d'ora in poi le stazioni non sono piu raggiungibili, non ha senso continuare oltre
            break;
        }else{//stazioni raggiungibili con la mia nuova auto (direzione: avanti)
            if(successive->maxPreStazione == NULL){
                successive->maxPreStazione = miaStaz;
            }else{
                if(successive->maxPreStazione->km > miaStaz->km){
                    successive->maxPreStazione = miaStaz;
                }
            }
        }
    }
    //aggiornamento delle stazioni precedenti
    while(precedenti->preStazione != NULL){
        precedenti = precedenti->preStazione;
        if((miaStaz->km - precedenti->km) > autonomia){//d'ora in poi le stazioni non sono piu raggiungibili, non ha senso continuare oltre
            break;
        }else{//stazioni raggiungibili con la mia nuova auto (direzione: indietro)
            if(precedenti->maxNextStazione == NULL){
                precedenti->maxNextStazione = miaStaz;
            }else{
                if(precedenti->maxNextStazione->km < miaStaz->km){
                    precedenti->maxNextStazione = miaStaz;
                }
            }
        }
    }
    return 1;
}
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

    if(precedenti == NULL || successive == NULL) return 2;//just added. (if no station is available how can we add a car??)

    if(successive->km > x) successive = successive->preStazione;
    if(precedenti->km < y) precedenti = precedenti->nextStazione;
    //fine conf iniziale
    while(successive != miaStaz){
        if(successive->maxPreStazione != NULL){
            if(successive->maxPreStazione->km <= z) break;
        }
        successive->maxPreStazione = miaStaz;
        successive = successive->preStazione;
    }
    while(precedenti != miaStaz){
        if(precedenti->maxNextStazione != NULL){
            if(precedenti->maxNextStazione->km >= z) break;
        }
        precedenti->maxNextStazione = miaStaz;
        precedenti = precedenti->nextStazione;
    }
    return 1;
}
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
int removedMaxAuto2(stazione* staz, int oldAutonomia){//this function can be void
    stazione* miaStaz = NULL;
    stazione* successive = NULL;
    stazione* precedenti = NULL;
    miaStaz = staz;
    successive = staz;
    precedenti = staz;
    //printf("you are removing a top car\n");
    //adjusting next station properties
    while(successive->nextStazione != NULL){
        successive = successive->nextStazione;
        if((successive->km - miaStaz->km) > oldAutonomia){//non ci arrivavo prima, di sicuro nemmeno ora che un auto in meno
            break;
        }else{
            if((successive->km - miaStaz->km) > miaStaz->autoMax){
                if(successive->maxPreStazione == miaStaz){
                    aggiornaMaxPrevStaz(miaStaz, successive);
                    //aggiornaMaxPrev(successive, miaStaz);
                }
            }
        }
    }
    //adjusting previous station properties
    while(precedenti->preStazione != NULL){
        precedenti = precedenti->preStazione;
        if((miaStaz->km - precedenti->km) > oldAutonomia){
            break;
        }else{
            if((miaStaz->km - precedenti->km) > miaStaz->autoMax){
                if(precedenti->maxNextStazione == miaStaz){
                    aggiornaMaxNextStaz(miaStaz, precedenti);
                    //aggiornaMaxNext(successive, miaStaz);
                }
            }
        }
    }
    return 1;
}
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

    if(precedenti == NULL || successive == NULL) return 2;//just added. (if no station is available how can we add a car??)

    if(successive->km > x) successive = successive->preStazione;
    if(precedenti->km < y) precedenti = precedenti->nextStazione;
    //fine conf iniziale
    while(successive != miaStaz){
        if(successive->maxPreStazione != NULL){//usless
            if(successive->maxPreStazione->km < z) break;
        }
        if((successive->km - miaStaz->km) > miaStaz->autoMax){
            aggiornaMaxPrevStaz(miaStaz, successive);
        }else{//le prossime stazioni le raggiungevo e le raggiungo ancora
            break;
        }
        successive = successive->preStazione;
    }
    while(precedenti != miaStaz){
        if(precedenti->maxNextStazione != NULL){//ulsess
            if(precedenti->maxNextStazione->km > z) break;
        }
        if((miaStaz->km - precedenti->km) > miaStaz->autoMax){
            aggiornaMaxNextStaz(miaStaz, precedenti);
        }else{//le prossime stazioni le raggingevo e le raggiungo ancora
            break;
        }
        precedenti = precedenti->nextStazione;
    }
    return 1;
}

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
void stampaTappe(){
    int i;
    for(i=0;i<indiceTappe-1;i++){
        printf("%d ", tappe[i]);
    }
    printf("%d\n", tappe[indiceTappe-1]);
}
void swapTappe(int indice, int km){
    tappe[indice] = km;
}


//aggiungiStazione Utils
int aggiornaMaxNext(stazione* arrivo, stazione* partenza){
    //backward direction of march
    if(arrivo != partenza){
        if((partenza->km - arrivo->km) <= partenza->autoMax){
            arrivo->maxNextStazione = (struct stazione*) partenza;
            return 1;
        }else{
            if(partenza->preStazione!=NULL){
                return aggiornaMaxNext(arrivo, (stazione *) partenza->preStazione);
            }
            return 1;
        }
    }else{//arrivo == partenza
        //arrivo->maxNextStazione = NULL;
        return 0;
    }
}
int aggiornaMaxPrev(stazione* arrivo, stazione* partenza){
    //straight direction of march
    if(arrivo != partenza){
        if((arrivo->km - partenza->km) <= partenza->autoMax){
            arrivo->maxPreStazione = (struct stazione*) partenza;
            return 1;
        }else{
            if(partenza->nextStazione!=NULL){
                return aggiornaMaxPrev(arrivo, (stazione *) partenza->nextStazione);
            }
            return 1;
        }
    }else{//partenza == arrivo
        //arrivo->maxPreStazione = NULL;
        return 0;
    }
}
int aggiornaDipendenze(stazione* staz){//this function can be void
    //configuring MaxNext
    if(staz->nextStazione != NULL){
        if(((stazione*)(staz->nextStazione))->maxNextStazione != NULL){//next station can be reached backwards from someone
            aggiornaMaxNext(staz, (stazione *) ((stazione *) (staz->nextStazione))->maxNextStazione);//check if that someone can reach me (staz) too
        }else{//next station cant be reached from anyone backwords: this means the only one station that can reach my staz backwords CAN (not mandatory) is the next one
            aggiornaMaxNext(staz, (stazione *) staz->nextStazione);
        }
    }//if staz->nextStazione == NULL: staz is the last station: for sure there isnt an other station after that that can reach our freshlyu added (last) station
    //configuring MaxPrev
    if(staz->preStazione != NULL){
        if(((stazione*)(staz->preStazione))->maxPreStazione != NULL){//prev station can be reached from someone behind
            aggiornaMaxPrev(staz, (stazione *) ((stazione *) (staz->preStazione))->maxPreStazione);//check if that someone can reach me (staz) too
        }else{//prev station cant be reached from anyone: this means the only one station that can reach my staz CAN (not mandatory) is the previous one
            aggiornaMaxPrev(staz, (stazione *) staz->preStazione);
        }
    }//if staz->preStazione == NULL: staz is the first station: for sure there isnt an other station before that that can reach our freshlyu added (first) station
    return 1;
}

//removedMaxAuto Utils. (nb those functions need a correct ad updated data structure to work properly). NB non passare mai stessa stazione
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
        if(partenza->nextStazione != NULL){//usless control?
            if(partenza->nextStazione == arrivo){
                arrivo->maxPreStazione = NULL;
                return 0;
            }
            return aggiornaMaxPrevStaz(partenza->nextStazione, arrivo);
        }
        return 0;//technically, we should never reach this point: it means partenza->nextStazione == NULL and we havent
        // incontered arrivo yet. since its a ordered list and arrivo comes after partenza (in this case) this cant happen.
    }
}
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
        if(partenza->preStazione != NULL){
            if(partenza->preStazione == arrivo){
                arrivo->maxNextStazione = NULL;
                return 0;
            }
            return aggiornaMaxNextStaz(partenza->preStazione, arrivo);
        }
        return 0;//technically, we should never reach this point: it means partenza->preStazione == NULL and we havent
        // incontered arrivo yet. since its a ordered list and arrivo comes before partenza (in this case) this cant happen.
    }
}

void initializeTable() {
    int i;
    for (i = 0; i < dimAshTable; i++) {
        table[i].miaStazione = NULL;
        table[i].sameAsh = NULL;
    }
}//ok
void initializeStation(stazione* s) {
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
void initializeBucket(nodoAshTable* n){
    n->miaStazione = NULL;
    n->sameAsh = NULL;
}
void initializeTappe(){
    int i;
    for(i = 0; i < dimVettoreTappe; i++){
        tappe[i] = -1;
    }
    indiceTappe = 0;
}


void gestioneFlussoEsecuzione() {
    initializeTable();
    initializeTappe();
    inizializzaVettore(&inizio);
    inizializzaStringa();
    char c, Kstaz;
    int i, supp, Staz;
    while ((c = getchar()) != EOF) {//modficato per DEBUG!
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
                                        continue;//non ci importa il numero di auto, ma solo la loro autonomia (inn realta
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
    //stampaStazioni();//debug
    //verificaStazioni();//debug
}//ok

//debug functions
void stampaStazioni() {
    stazione *start = sIniziale;
    int cont = 0;
    while (start->nextStazione != NULL) {
        start = (stazione *) start->nextStazione;
        printf("stazione numero %d al chilometro %d\n", cont, start->km);
        cont++;
    }
}

void verificaStazioni() {//ancora fatto male con versione vecchia, migliorabile
    stazione *start = sIniziale;
    int trovata = 0;
    nodoAshTable *app = 0;
    int cont = 0;
    while (start->nextStazione != NULL) {
        start = (stazione *) start->nextStazione;
        if (table[ash(start->km)].miaStazione != NULL) {
            if (start->km == table[ash(start->km)].miaStazione->km) {//prima entry
                trovata = 1;
            }
        }
        app = (nodoAshTable *) table[ash(start->km)].sameAsh;//oppure innestata nei bucket
        while (app != NULL) {
            if (app->miaStazione != NULL) {
                if (start->km == app->miaStazione->km) {
                    trovata = 1;
                }
            }
            app = (nodoAshTable *) app->sameAsh;
        }
        if (trovata == 0) {
            printf("impossibile accedere alla stazione %d dalla ashTable\n", start->km);
            cont++;
        }
        trovata = 0;
    }
    printf("mancano all appello %d stazioni", cont);
}


int main() {
    gestioneFlussoEsecuzione();
    return 0;
}