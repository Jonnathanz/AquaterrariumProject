/*
 * File:   troca_agua_2.h
 * Author: Jonnathan Alves
 *
 * Created on 12 de Fevereiro de 2021, 22:03
 * 
 * Configuração das válvulas e funções para o Sistema de troca de água
 * 
 * Iluminacao é programada para ficar 20 minutos ligado a partir do horário
 * pré-definido
 */

// DEFINIÇÕES___________________________________________________________________
#define valvula1 PORTCbits.RC6    // Válvula para saída de água    (saída)
#define valvula2 PORTCbits.RC7    // Válvula para entrada de água  (saída)
#define besvazia PORTBbits.RB2    // Botão para o acionamento dos esvaziamento
#define benche   PORTBbits.RB1    // Botão para o acionamento do enchimento

// CONSTANTES GLOBAIS___________________________________________________________
unsigned int ativoTrocaaAgua;      // 1 - O sistema está ativo; 0 - O sistema está inativo
float nvlAgua;

// PROTÓTIPOS___________________________________________________________________
void iniciar_troca_agua();
void trocar_agua();

// CONFIGURA A TROCA DE ÁGUAS___________________________________________________
void iniciar_troca_agua(){
    TRISCbits.RC6 = 0;// SAÍDA
    TRISCbits.RC7 = 0;// SAÍDA
    
    TRISBbits.RB2 = 1;// ENTRADA
    TRISBbits.RB1 = 1;// ENTRADA
    
    valvula1 = 0;
    valvula2 = 0;
    ativoTrocaaAgua = 0;
    //distancia_2 = 17;
}

// LIGA A LAMPADA NO HORARIO ESPECIFICADO_______________________________________
void trocar_agua(){
    if(!sist_trocaAgua_on){ 
        // Caso o sistema de troca da água esteja desativado
        valvula1 = 0;
        valvula2 = 0;
        return;
    }
    if(besvazia || benche){
        distance_2();
        // distancia 2 varia de 16cm até 34 cm
        // esquema de medição de nvl parecido com o da ração
        nvlAgua = (distancia_2 - 34)/17;
    }
    if(besvazia && (distancia_2 <= 17)){ 
        // Esvazia o aquaterrário abrindo a válvula de saída de água
        valvula1 = 1;
        valvula2 = 0;
        ativoTrocaaAgua = 1;
    } 
    if((distancia_2 >= 32) && valvula1 == 1) { 
        // Fecha a válvula de saída após o esvaziamento do aquaterrário
        valvula1 = 0;
        valvula2 = 0;
        ativoTrocaaAgua = 0;
    }
    if(benche && (distancia_2 >= 30)){
        // Preenche de água o aquaterrário abrindo a válvula de entrada de água
        valvula1 = 0;
        valvula2 = 1;
        ativoTrocaaAgua = 1;
    }
    if((distancia_2 <= 17) && valvula2 == 1) { 
        // Fecha a válvula de saída após o esvaziamento do aquaterrário
        valvula1 = 0;
        valvula2 = 0;
        ativoTrocaaAgua = 0;
    }
    if(ativoTrocaaAgua) {
        distance_2();    // Inicia a medição do echo2
        nvlAgua = (34 - distancia_2)/17*100;
    }
}