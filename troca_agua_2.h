/*
 * File:   troca_agua_2.h
 * Author: Jonnathan Alves
 *
 * Created on 12 de Fevereiro de 2021, 22:03
 * 
 * Configura��o das v�lvulas e fun��es para o Sistema de troca de �gua
 * 
 * Iluminacao � programada para ficar 20 minutos ligado a partir do hor�rio
 * pr�-definido
 */

// DEFINI��ES___________________________________________________________________
#define valvula1 PORTCbits.RC6    // V�lvula para sa�da de �gua    (sa�da)
#define valvula2 PORTCbits.RC7    // V�lvula para entrada de �gua  (sa�da)
#define besvazia PORTBbits.RB2    // Bot�o para o acionamento dos esvaziamento
#define benche   PORTBbits.RB1    // Bot�o para o acionamento do enchimento

// CONSTANTES GLOBAIS___________________________________________________________
unsigned int ativoTrocaaAgua;      // 1 - O sistema est� ativo; 0 - O sistema est� inativo

// PROT�TIPOS___________________________________________________________________
void iniciar_troca_agua();
void trocar_agua();

// CONFIGURA A TROCA DE �GUAS___________________________________________________
void iniciar_troca_agua(){
    TRISCbits.RC6 = 0;// SA�DA
    TRISCbits.RC7 = 0;// SA�DA
    
    TRISBbits.RB2 = 1;// ENTRADA
    TRISBbits.RB1 = 1;// ENTRADA
    
    valvula1 = 0;
    valvula2 = 0;
    ativoTrocaaAgua = 0;
    //distancia_2 = 17;
}

// LIGA A LAMPADA NO HORARIO ESPECIFICADO_______________________________________
void trocar_agua(){
    if(besvazia || benche){
        distance_2();
    }
    if(besvazia && (distancia_2 >= 17)){ 
        // Esvazia o aquaterr�rio abrindo a v�lvula de sa�da de �gua
        valvula1 = 1;
        valvula2 = 0;
        ativoTrocaaAgua = 1;
    } 
    if((distancia_2 >= 32) && valvula1 == 1) { 
        // Fecha a v�lvula de sa�da ap�s o esvaziamento do aquaterr�rio
        valvula1 = 0;
        valvula2 = 0;
        ativoTrocaaAgua = 0;
    }
    if(benche && (distancia_2 >= 30)){
        // Preenche de �gua o aquaterr�rio abrindo a v�lvula de entrada de �gua
        valvula1 = 0;
        valvula2 = 1;
    }
    if((distancia_2 <= 17) && valvula2 == 1) { 
        // Fecha a v�lvula de sa�da ap�s o esvaziamento do aquaterr�rio
        valvula1 = 0;
        valvula2 = 0;
        ativoTrocaaAgua = 0;
    }
    if(ativoTrocaaAgua) {
        distance_2();    // Inicia a medi��o do echo2
        sprintf2(buffer3, "distancia: ");
        nFloatToStr(buffer3+11, 1, distancia_2);
    }
    
    /*
    if(!(besvazia && benche)) {
        if(besvazia && distancia_2 <= 33.5){// Esvazia
            valvula1 = 1;
            valvula2 = 0;
            ativoTrocaaAgua = 1;
        }
        if(benche && distancia_2 >= 17.5){// Enche
            valvula1 = 0;
            valvula2 = 1;
            ativoTrocaaAgua = 1;
        }
    }  
    if((distancia_2 >= 33.5 && valvula1) || (distancia_2 <= 17.5 && valvula2)) {
        //Para de esvaziar ou encher
        valvula1 = 0;
        valvula2 = 0;
        ativoTrocaaAgua = 0;
        distancia_2 = 17.6;
        sprintf2(buffer2, "                ");
    }

    if(ativoTrocaaAgua) {
        distance_2();    // Inicia a medi��o do echo2
        sprintf2(buffer2, "distancia: ");
        nFloatToStr(buffer2+11, 1, distancia_2);
    }
     
     */
}
