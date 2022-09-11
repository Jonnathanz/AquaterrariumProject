/*
 * File:   string2.c
 * Author: Jonnathan Alves
 *
 * Created on 27 de Março de 2021, 15:2
 * 
 * BIBLIOTECA PARA A MANIPULAÇÃO DE STRINGS COM A FINALIDADE DE SUBSTITUIR 
 * A BIBLIOTECA stdio.h COM A FUNÇÃO sprinf
 */

// PROTÓTIPOS___________________________________________________________________
void nFloatToStr(char*, short, float);
void sprintf2(char *, char *);

// SUBROTINA PARA A ATRIBUIÇÃO DE UMA STRING PARA UMA VARIÁVEL__________________
void sprintf2(char *var, char *txt){
    /*
        Atribui uma string para uma variável de strings
        var: variável string a ser atribuído
        txt: texto a ser escrito na variável
    */

    int i = 0;
    while(txt[i] != '\0' && var[i] != '\0'){
        var[i] = txt[i];
        i++;
    }
}

// SUBROTINA PARA A CONVERSÃO DE UM VALOR FLOAT PARA STRING_____________________
void nFloatToStr(char *var, short n, float p){
    /*
        Atribui um valor float para uma string

        -> var: Variável string a ser atribuída
        -> n:   Qtde de números depois da virgula
        -> p:   valor float

        Se n=0: Pode fazer a conversão para inteiro
    */

    unsigned int i=0;
    int k=10;

    //VERIFICA INDIRETAMENTE A QTDE DE Nºs ANTES DA VIRGULA
    while(p/k >= 10)
        k = k*10;

    // RETIRA OS NÚMEROS ANTES DA VIRGULA
    while(k >= 1) {
        //printf("\t %f", p);
        var[i] = 48 + ((int) p/k);
        p = p - ((int) p/k)*k;
        k = k/10;
        i++;
    }

    // RETIRA OS NÚMEROS DEPOIS DA VIRGULA
    if(n > 0){
        var[i] = '.';
        for(k = i+1; k < (n + i + 1); k++){
            p = 10*p;
            var[k] = 48 + ((int) p);
            p = p - ((int) p);
        }
    }
}