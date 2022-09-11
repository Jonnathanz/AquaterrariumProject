/*
 * File:   main.c
 * Author: Jonnathan Alves
 *
 * Created on 12 de Fevereiro de 2021, 22:03
 * 
 * Configura��o e fun��es da iluminacao
 * 
 * Iluminacao � programada para ficar 20 minutos ligado a partir do hor�rio
 * pr�-definido
 */

// DEFINI��ES___________________________________________________________________
#define ilum PORTBbits.RB5 
#define hour_ilum 10 //Hor�rio que ir� ligar a l�mpada UVB + UVA (10:30)
#define min_ilum  30
#define sec_ilum  0
// CONSTANTES GLOBAIS___________________________________________________________

// PROT�TIPOS___________________________________________________________________
void configurar_ilum(void);
void liga_lampada(void);

// CONFIGURA A ILUMINA��O_______________________________________________________
void configurar_ilum(void){
    TRISBbits.RB5 = 0; // Definindo como sa�da o pino da ilumina��o
}
// LIGA A LAMPADA NO HORARIO ESPECIFICADO_______________________________________
void liga_lampada(){
    if(hour_ilum == hora_atual && min_ilum == minuto_atual && sec_ilum == segundo_atual){
        // LIGA A LAMPADA
        ilum = 1;
    } 
    if(minuto_atual == (min_ilum + 20)%60){ // DESLIGA A LAMPADA
        ilum = 0;
    }
}

