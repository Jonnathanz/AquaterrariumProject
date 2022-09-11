/*
 * File:   main.c
 * Author: Jonnathan Alves
 *
 * Created on 11 de Fevereiro de 2021, 13:07
 * Otimização do código on 17 de Março de 2021, 15:59
 * Segunda otimização do código em 01 de Setembro de 2021, 13:14
 * 
 * FIRMWARE DESENVOLVIDO COM A FINALIDADE DE AUTOMATIZAR OS SEGUINTES PROCESSOS
 * DE UM AQUATERRÁRIO:
 * 
 *      (a) Sistema de Temperatura                (X)
 *      (b) Sistema de Alimentação                (OK)
 *      (c) Sistema de Iluminação UVB + UVA       (OK)
 *      (d) Sistema de Troca de Água              (OK)
 * 
 * fclk  = 20 MHz
 */

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS______________________________________________

char buffer[21]  = "                    ";              // Primeira linha LCD
char buffer2[21] = "                    ";              // Segunda linha LCD
char buffer3[21] = "                    ";              // terceira linha LCD
char buffer4[21] = "                    ";              // Quarta linha LCD

// INCLUSÃO DE BIBLIOTECAS______________________________________________________
#include "string2.h"
#include "18F452_config_bits.h"
#include "I2C.h"
#include "ds1307.h"
#include "LCD_20x4.h"
#include "horario.h"
#include "iluminacao.h"
#include "hcsr04.h"
#include "troca_agua_2.h"

// DEFINIÇÕES___________________________________________________________________
#define emerg  PORTBbits.RB0  // BOTÂO DE EMERGÊNCIA
#define servo1 PORTBbits.RB7  // PRIMEIRO SERVO MOTOR
#define servo2 PORTBbits.RB6  // SEGUNDO SERVO MOTOR

#define horas_alim  9   // Horário que iniciará a alimentação 1x por dia
#define min_alim    0
#define sec_alim    0

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS______________________________________________

unsigned int alimentacao;       // 1 - horario de alimentar, 0 - ainda nao
unsigned int contadorTMR0;      // Verifica os PWM nos servos motores

unsigned int R1servo; // Usado para controle do tempo em high do servo motor 1
unsigned int R2servo; // Usado para controle do tempo em high do servo motor 2

// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void inicializa_interrupcoes(void); 
void horario_alimentacao(void);

// SUBROTINA PARA INTERRUPÇÕES__________________________________________________
void __interrupt() intAll(void) {
    // INTERRUPÇAO DO TIMER0: utilizado para o controle PWM dos servo motores
    if(TMR0IE && TMR0IF){ 
        TMR0IF  = 0; 
        if(!contadorTMR0){
            // Modula o PWM do primeiro servo-motor nos primeiros 10ms
            servo2 = 0;
            if(servo1){
                // mantém nível lógico baixo do servo1
                TMR0L   = 315 - R1servo;
                servo1  = 0;
                contadorTMR0 = ~contadorTMR0;
            }else{
                // mantém nível lógico alto do servo1
                TMR0L   = R1servo;
                servo1  = 1;
            }
        } else {
            // Modula o PWM do segundo servo-motor nos 10ms finais
            servo1 = 0;
            if(servo2){
                // mantém nível lógico baixo do servo2
                TMR0L   = 315 - R2servo;
                servo2  = 0;
                contadorTMR0 = ~contadorTMR0;
                temporizacao();
            }else{
                // mantém nível lógico alto do servo2
                TMR0L   = R2servo;
                servo2  = 1;
            } 
        }
    }
    
    // INTERRUPÇÃO DO CCP1: Utilizado para o echo no hcsr_1
    if(CCP1IF && CCP1IE) {
        if(CCP1CON == 0b00000101) {  
            // Habilitação por borda de subida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
            CCP1IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP1CON     = 0b00000100;   // Configura a borda de captura para descida
            CCP1IE      = 1;            // Habilita a interrupção do CCP1
            TMR1H       = 0x00;         // Zeram os registradores do timer1
            TMR1L       = 0x00;
            TMR1ON      = 1;            // Habilita o timer1
        } else {
            // Habilitação por borda de descida
            tempo_h1    = CCPR1H;       // Carrengando os registradores de tempo
            tempo_l1    = CCPR1L;
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
            TMR1ON      = 0;            // Desabilita o timer1
            CCP1IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP1CON     = 0b00000101;   // Configura a borda de captura para subida
        }
    }
    
    // INTERRUPÇÃO DO CCP2: Utilizado para o echo no hcsr_2
    if(CCP2IF && CCP2IE) {
        if(CCP2CON == 0b00000101) {  
            // Habilitação por borda de subida
            CCP2IF      = 0;            // Limpa a flag para uma nova captura
            CCP2IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP2CON     = 0b00000100;   // Configura a borda de capitura para descida
            CCP2IE      = 1;            // Habilita a interrupção do CCP1
            TMR3H       = 0x00;         // Zeram os registradores do timer1
            TMR3L       = 0x00;
            TMR3ON      = 1;            // Habilita o timer1
        } else {
            // Habilitação por borda de descida
            tempo_h2    = CCPR2H;       // Carrengando os registradores de tempo
            tempo_l2    = CCPR2L;
            CCP2IF      = 0;            // Limpa a flag para uma nova captura
            TMR3ON      = 0;            // Desabilita o timer1
            CCP2IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP2CON     = 0b00000101;   // Configura a borda de capitura para subida
        }
    }
    
    // INTERRUPÇÃO DO INT 0: Utilizado para habilitar o botão de emergência
    if(INT0E && INT0F) { 
        INT0F = 0;      // Limpa a flag para uma nova captura de INT0
    }
}

// ROTINA PRINCIPAL_____________________________________________________________
void main(void) {
    TRISBbits.RB7 = 0;
    TRISBbits.RB6 = 0;
    
    contadorTMR0    = 0;
    servo1          = 0;
    servo2          = 0;
    
    R1servo = 227;  // 90°
    R2servo = 227;  // 0°
    
    TRISBbits.RB0 = 1;
    alimentacao = 0;
    
    Inicializa_LCD();                   // Configura o display LCD 16x2
    inicializa_hcsr04();                // Configura os sensores de ultrassom
    configurar_horario();               // Configura o temporizador
    configurar_ilum();                  // Configura a iluminação
    iniciar_troca_agua();               // Configura o sist de troca de água
    inicializa_interrupcoes();          // Configura as interrupções
    I2C_init(100000);                   // Inicializa a comunicação I2C (100kHz)
    
    __delay_ms(1000); 
    Write_time(10, 4, 17);
    Read_time(buffer3);

    while(1) {
        
        horario_alimentacao();
        liga_lampada();
        Escreve_texto_com_pos(1,1,buffer);
        Escreve_texto_com_pos(2,1,buffer2);
        Escreve_texto_com_pos(3,1,buffer3);
        Escreve_texto_com_pos(4,1,buffer4);
        //trocar_agua();
        __delay_ms(300); 
    }
    
    return;
}

// SUBROTINAS PARA A INICIALIZAÇÃO DAS INTERRUPÇÕES_____________________________
void inicializa_interrupcoes(void) {
    INTCON  = 0b11110000;
    INTCON2 = 0b11000000;
    
    // CONFIGURAÇÃO DA INTERRUPÇÃO DO TIMER0 (Para controle de servo motores)
    T0CON   = 0b11000111;
    
    // CONFIGURAÇÃO DA INTERRUPÇÃO DO CCP1 E CCP2 (Para operação doS sensores de ultrassom)

    IPEN        = 0;
    TMR1IE      = 0;            // desabilita a interrupcao do timer1
    TMR3IE      = 0;
    CCP1IE      = 0;            // Desabilita a interrupção por captura  
    CCP2IE      = 0;
    
    T1CON       = 0b10100000;   // Configuração do timer1
    T3CON       = 0b10101000;   // MUDAR DEPOIS
    
    CCP2CON     = 0b00000101; 
    CCP1CON     = 0b00000101;   // Configura a borda de captura para subida
    
    // CONFIGURAÇÃO DA INTERRUPÇAO DO INT0 (Para botão de emergência)
    // INTCON2bits.INTEDG0 = 1; 
    // INTCONbits.INT0F = 0; 
    // INTCONbits.INT0E = 1;
    // INTCONbits.GIE = 1;
    
    ei();
}

// SUBROTINA PARA A VERIFICACAO E REALIZACAO DO SISTEMA DE ALIMENTACAO__________
void horario_alimentacao(void){
    /*
     *      Faz a medição do nível de ração e Abre os servos motores quando
     *      estiver no horário solicitado para a alimentação
     */

    if(!alimentacao && hora_atual == horas_alim && minuto_atual == min_alim && segundo_atual == sec_alim) {
        distance_1();   //  REVER ESSE IF DEPOIS
    } else if (!alimentacao && hora_atual == horas_alim && minuto_atual == min_alim && segundo_atual < sec_alim+2) {
        distance_1();
        if(distancia_1 >= 14) {
            sprintf2(buffer2, "R.RACAO VAZIO   ");
        } else {
            sprintf2(buffer2, "distancia: ");
            nFloatToStr(buffer2+11, 1, distancia_1);
            alimentacao = 1;
        }
    }
    else if (alimentacao == 1 && segundo_atual < sec_alim+7) {
        R1servo = 237;  // 90°
    } else if (alimentacao == 1 && segundo_atual < sec_alim+11) {
        R1servo = 227;  // 90°
    } else if (alimentacao == 1 && segundo_atual < sec_alim+16) {
        R2servo = 237;  // 90º
    } else {
        R2servo = 227;  // 0°
        alimentacao = 0;
        if(segundo_atual == sec_alim+16)
            sprintf2(buffer2, "               ");
    }
}