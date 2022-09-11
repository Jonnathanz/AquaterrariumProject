/*
 * File:   string2.c
 * Author: Jonnathan Alves
 *
 * Created on 27 de Mar�o de 2021, 15:2
 * 
 * BIBLIOTECA PARA A MANIPULA��O DE STRINGS COM A FINALIDADE DE SUBSTITUIR 
 * A BIBLIOTECA stdio.h COM A FUN��O sprinf
 */

// PROT�TIPOS___________________________________________________________________
void nFloatToStr(char*, short, float);
void sprintf2(char *, char *);

// SUBROTINA PARA A ATRIBUI��O DE UMA STRING PARA UMA VARI�VEL__________________
void sprintf2(char *var, char *txt){
    /*
        Atribui uma string para uma vari�vel de strings
        var: vari�vel string a ser atribu�do
        txt: texto a ser escrito na vari�vel
    */

    int i = 0;
    while(txt[i] != '\0' && var[i] != '\0'){
        var[i] = txt[i];
        i++;
    }
}

// SUBROTINA PARA A CONVERS�O DE UM VALOR FLOAT PARA STRING_____________________
void nFloatToStr(char *var, short n, float p){
    /*
        Atribui um valor float para uma string

        -> var: Vari�vel string a ser atribu�da
        -> n:   Qtde de n�meros depois da virgula
        -> p:   valor float

        Se n=0: Pode fazer a convers�o para inteiro
    */

    unsigned int i=0;
    int k=10;

    //VERIFICA INDIRETAMENTE A QTDE DE N�s ANTES DA VIRGULA
    while(p/k >= 10)
        k = k*10;

    // RETIRA OS N�MEROS ANTES DA VIRGULA
    while(k >= 1) {
        //printf("\t %f", p);
        var[i] = 48 + ((int) p/k);
        p = p - ((int) p/k)*k;
        k = k/10;
        i++;
    }

    // RETIRA OS N�MEROS DEPOIS DA VIRGULA
    if(n > 0){
        var[i] = '.';
        for(k = i+1; k < (n + i + 1); k++){
            p = 10*p;
            var[k] = 48 + ((int) p);
            p = p - ((int) p);
        }
    }
}