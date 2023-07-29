#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

typedef struct cifra{
    unsigned x:1;
    void * nxt;
} cifra;

typedef struct vettoreCifre{
    cifra c[10];
    struct vettoreCifre * padre;
    char cont;
} vettoreCifre;

char stringa[10];
vettoreCifre inizio;
int contatore3 = 0;
vettoreCifre * inizioMemVettori = NULL;
vettoreCifre * fineMemVettori = NULL;
vettoreCifre * sonoAllaFine3 = NULL;

int ricercaDS(vettoreCifre * v, int cifra, int step, stazione* s);
int creaDS(vettoreCifre * v, int cifra, int step, stazione* s);
int riparaEliminazione(vettoreCifre * v, int k);
stazione * ricercaVicinoDSVera(int kilometro, vettoreCifre * v, int cifra, int step);
stazione* ricercaMaxDS(vettoreCifre * v, int c);
stazione* ricercaMinDS(vettoreCifre * v, int c);

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
    return riparaEliminazione(s->puntDS, s->km/10);
}

int riparaEliminazione(vettoreCifre * v, int k){
    if(v->cont == 0 && v->padre != NULL){
            v->padre->c[k%10].nxt = NULL;
            v->padre->c[k%10].x = 0;
            v->padre->cont = (char)(int)(v->padre->cont -1);
            return riparaEliminazione(v->padre, k/10);
        }else{
            return 1;
        }
}

//NB non chiamare la funzione se la struttura dati è vuota o incosistente, altrimenti non funziona!
//NB non chiamare la funzione per cercare una stazione esistente! passa il kilometro a cui non esistono stazioni!
stazione * ricercaVicinoDS(int kilometro){
    int km = kilometro;
    int c = 0;
    inizializzaStringa();
    c = nCifre(km);
    aggiornaStringa(km, c);
    return ricercaVicinoDSVera(km, &inizio, c, c);
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

int main(){
    inizializzaVettore(&inizio);
    inizializzaStringa();
    inizio.cont =  (char)((int)inizio.cont + 1);
    inizio.cont =  (char)((int)inizio.cont - 1);
    aggiornaStringa(12345, nCifre(12345));
    printf("numero cifre di 10 è: %d \n", nCifre(10));
    printf("numero cifre di 7 è: %d \n", nCifre(7));
    printf("numero cifre di 0 è: %d \n", nCifre(0));
    printf("numero cifre di 987546324 è: %d \n", nCifre(987546324));
    stazione s1, s2, s3, s4, s5, s6, s7, s8, s9 ,s10;
    s1.km = 12345;
    s4.km = 12349;
    s5.km = 12355;
    s6.km = 19445;
    s2.km = 70070000;
    s3.km = 0;
    s1.puntDS = NULL;
    s2.puntDS = NULL;
    s3.puntDS = NULL;
    stazione * p = NULL;
    p = &s1;
    aggiungiStazioneDS(p->km, p);
    p = &s4;
    aggiungiStazioneDS(p->km, p);
    p = &s5;
    aggiungiStazioneDS(p->km, p);
    p = &s6;
    aggiungiStazioneDS(p->km, p);
    p = &s2;
    aggiungiStazioneDS(p->km, p);
    p = &s3;
    aggiungiStazioneDS(p->km, p);

    p = &s1;
    //eliminaDS(p);
    //eliminaDS(&s4);

    s7.km = 12340;
    s8.km = 12319;
    s9.km = 15655;
    s10.km = 194754789;
    aggiungiStazioneDS(s7.km, &s7);
    aggiungiStazioneDS(s8.km, &s8);
    aggiungiStazioneDS(s9.km, &s9);
    aggiungiStazioneDS(s10.km, &s10);
    printf("il contatore ha contato: %d entryes\n", (int)inizio.cont);



    printf("kilometro fornito ------>> kilometro stazione piu vicina\n");
    printf("%d ------>> %d\n", s1.km-10, ricercaVicinoDS(s1.km-10)->km);
    printf("%d ------>> %d\n", s1.km-100, ricercaVicinoDS(s1.km-100)->km);
    printf("%d ------>> %d\n", s1.km-23, ricercaVicinoDS(s1.km-23)->km);
    printf("%d ------>> %d\n", s5.km-12, ricercaVicinoDS(s5.km-12)->km);
    printf("%d ------>> %d\n", s1.km-7000, ricercaVicinoDS(s1.km-7000)->km);

    printf("%d ------>> %d\n", s1.km+2, ricercaVicinoDS(s1.km+2)->km);
    printf("%d ------>> %d\n", s1.km+3, ricercaVicinoDS(s1.km+3)->km);
    eliminaDS(&s5);
    printf("%d ------>> %d\n", s1.km+5, ricercaVicinoDS(s1.km+5)->km);
    printf("%d ------>> %d\n", s1.km-50, ricercaVicinoDS(s1.km-50)->km);
    printf("%d ------>> %d\n", s3.km+1, ricercaVicinoDS(s3.km+1)->km);
    printf("%d ------>> %d\n", s3.km+70000000, ricercaVicinoDS(s3.km+70000000)->km);
    return 0;
}