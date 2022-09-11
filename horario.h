/*
 * File:   main.c
 * Author: Jonnathan Alves
 *
 * Created on 13 de Fevereiro de 2021, 10:47
 * 
 * BIBLIOTECA PARA A CONFIGURAÇÃO E EXECUÇÃO DE HORÁRIO E TEMPORIZAÇÃO
 * 
 * clock: 20 MHz
 */

// DEFINIÇÕES___________________________________________________________________
// As horas devem ser configuradas a cada reset do microcontrolador
#define hora_a 8        // HORA ATUAL
#define min_a  59       // MINUTO ATUAL
#define sec_a  55       // SEGUNDO ATUAL

#define k_1s 50      // constante de tempo para 1s;
// CONSTANTES GLOBAIS___________________________________________________________

unsigned int hora_atual;    
unsigned int minuto_atual;
unsigned int segundo_atual;

int k_temp;

// PROTÓTIPOS___________________________________________________________________
void configurar_horario(void);
void temporizacao(void);

// FUNÇÃO QUE CONFIGURA O HORÁRIO ATUAL_________________________________________
void configurar_horario(void){
    hora_atual = hora_a;
    minuto_atual = min_a;
    segundo_atual = sec_a;
    k_temp = 0;
}

// FUNÇÃO QUE REALIZA A TEMPORIZAÇÃO____________________________________________
void temporizacao(void) {
    if(k_temp > k_1s){          // CONTAGEM DOS SEGUNDOS
        sprintf2(buffer, "        :  :       ");
        nFloatToStr(buffer+6, 0, hora_atual);
        nFloatToStr(buffer+9, 0, minuto_atual);
        nFloatToStr(buffer+12, 0, segundo_atual);
        
        segundo_atual++;
        k_temp = 0;
    }
    if(segundo_atual > 59){     // CONTAGEM DOS MINUTOS
        minuto_atual++;
        segundo_atual = 0;
    }
    if(minuto_atual > 59){      // CONTAGEM DAS HORAS
        hora_atual++;
        minuto_atual = 0;
    }       
    if(hora_atual > 23){        // ZERA AS HORAS COM O PASSAR DE UM DIA
        hora_atual = 0;
    }
    
    k_temp++;
}
