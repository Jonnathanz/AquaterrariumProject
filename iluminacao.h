/*
 * File:   main.c
 * Author: Jonnathan Alves
 *
 * Created on 12 de Fevereiro de 2021, 22:03
 * 
 * Configuração e funções da iluminacao
 * 
 * Iluminacao é programada para ficar 20 minutos ligado a partir do horário
 * pré-definido
 */

// DEFINIÇÕES___________________________________________________________________
#define ilum PORTDbits.RD1

//Horário que irá ligar a lâmpada UVB + UVA (10:30)
unsigned char hour_ilum = 10;
unsigned char min_ilum  = 30;
unsigned char sec_ilum  = 0;
// CONSTANTES GLOBAIS___________________________________________________________

// PROTÓTIPOS___________________________________________________________________
void configurar_ilum(void);
void liga_lampada(void);

// CONFIGURAÇÃO DA ILUMINAÇÃO___________________________________________________
void configurar_ilum(void){
    TRISDbits.RD1 = 0; // Definindo como saída o pino da iluminação
}
// LIGA A LAMPADA NO HORARIO ESPECIFICADO_______________________________________
void liga_lampada(){
    if(!sist_ilum_on){
        // Caso o sistema de iluminação esteja desativado
        ilum = 0;
        return;
    }
    if(hour_ilum == hora_atual && min_ilum == minuto_atual && sec_ilum == segundo_atual){
        // LIGA A LAMPADA
        ilum = 1;
    } 
    if(minuto_atual == (min_ilum + 20)%60){ 
        // DESLIGA A LAMPADA
        ilum = 0;
    }
}