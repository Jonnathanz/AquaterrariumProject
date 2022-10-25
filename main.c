/*
 * File:   main.c
 * Author: Jonnathan Alves
 *
 * Created on 11 de Fevereiro de 2021, 13:07
 * Otimiza��o do c�digo on 17 de Mar�o de 2021, 15:59
 * Segunda otimiza��o do c�digo em 01 de Setembro de 2021, 13:14
 * 
 * FIRMWARE DESENVOLVIDO COM A FINALIDADE DE AUTOMATIZAR OS SEGUINTES PROCESSOS
 * DE UM AQUATERR�RIO:
 * 
 *      (a) Sistema de Temperatura                (OK)
 *      (b) Sistema de Alimenta��o                (OK)
 *      (c) Sistema de Ilumina��o UVB + UVA       (OK)
 *      (d) Sistema de Troca de �gua              (OK)
 * 
 * fclk  = 20 MHz
 * 
 * Conex�es:
 *      1  - MCLR_/VPP:  Bot�o de Reset
 *      2  - RA0:        Bot�o IHM "cursor para cima"  (Entrada)
 *      3  - RA1:        Bot�o IHM "cursor para baixo" (Entrada)
 *      4  - RA2:        Bot�o IHM "confirmar"         (Entrada)
 *      5  - RA3:        Bot�o IHM "sair"              (Entrada)
 *      6  - RA4:        Conex�o OneWire com o DS18B20 (Entrada/Sa�da)
 *      13 - OSC1_/CLK1: Primeira conex�o com o circuito oscilador
 *      14 - OSC2_/CLK2: Segunda conex�o com o circuito oscilador
 *      15 - RC0:        Trigger do HCSR04 do sistema de alimenta��o (Sa�da)
 *      16 - RC1:        Echo do HCSR04 do sistema de troca de �gua  (Entrada)
 *      17 - RC2:        Echo do HCSR04 do sistema de alimenta��o    (Sa�da)
 *      18 - RC3/SCL:    Conex�o SCL da comunica��o I�C com o DS1307 (Sa�da)
 *      19 - RD0:        Trigger do HCSR04 do sistema de troca de �gua (Sa�da)
 *      20 - RD1:        Conex�o com a l�mpada UVA/UVB (Sa�da)
 *      21 - RD2:        Conex�o RS do display LCD 20x4  (Sa�da)
 *      22 - RD3:        Conex�o E do display LCD 20x4   (Sa�da)
 *      23 - RC4/SDA:    Conex�o SDA da comunica��o I�C com o DS1307 (E/S) 
 *      24 - RC5:        Conex�o com a resist�ncia de aquecimento de �gua (S)
 *      25 - RC6:        Conex�o com a v�lvula solen�ide de sa�da de �gua (S)
 *      26 - RC7:        Conex�o com a v�lvula solen�ide de entrada de �gua (S)
 *      27 - RD4:        Conex�o D4 do display LCD 20x4  (Sa�da)
 *      28 - RD5:        Conex�o D5 do display LCD 20x4  (Sa�da)
 *      29 - RD6:        Conex�o D6 do display LCD 20x4  (Sa�da)
 *      30 - RD7:        Conex�o D7 do display LCD 20x4  (Sa�da)
 *      33 - RB0/INT0:   Bot�o de emerg�ncia (Entrada)
 *      34 - RB1:        Bot�o para a entrada de �gua  (Entrada)
 *      35 - RB2:        Bot�o para a sa�da de �gua    (Entrada)
 *      
 *      39 - RB6:        Conex�o PWM com o segundo servo-motor  (Sa�da)
 *      40 - RB7:        Conex�o PWM com o primeiro servo-motor (Sa�da)
 *      
 */

// DECLARA��O DE VARI�VEIS GLOBAIS______________________________________________
char buffer[21]       = "       :  :         ";     // Primeira linha LCD
char bufferNvlAgua[5] = "    ";                     // Buffer para nvl da �gual
char bufferAlim[5]    = "    ";                     // Buffer para alimenta��o
char bufferTemp[6]    = "     ";                    // Buffer para temperatura
char bufferConfig[21]     = "                    "; // Buffer para configura��o

// Hor�rio definido para a configura��o inicial
unsigned int hora_atual    = 0x08;                  // Hora atual
unsigned int minuto_atual  = 0x59;                  // Minuto atuall
unsigned int segundo_atual = 0x55;                  // Segundo atual

unsigned char sist_temp_on      = 1;    // Sistema de temperatura   ON/OFF
unsigned char sist_ilum_on      = 1;    // Sistema de ilumina�ao    ON/OFF
unsigned char sist_alim_on      = 1;    // Sistema de alimenta��o   ON/OFF
unsigned char sist_trocaAgua_on = 1;    // Sistema de troca de �gua ON/OFF

// INCLUS�O DE BIBLIOTECAS______________________________________________________
#include "string2.h"
#include "18F452_config_bits.h"
#include "ds18b20.h"
#include "LCD_20x4.h"
#include "iluminacao.h"
#include "hcsr04.h"
#include "troca_agua_2.h"
#include "ds1307.h"

// DEFINI��ES___________________________________________________________________
#define emerg      PORTBbits.RB0        // BOT�O DE EMERG�NCIA
#define servo1     PORTBbits.RB7        // PRIMEIRO SERVO MOTOR
#define servo2     PORTBbits.RB6        // SEGUNDO SERVO MOTOR
#define bup        PORTAbits.RA0        // BOT�O PARA SUBIDA DO CURSOR
#define bdown      PORTAbits.RA1        // BOT�O PARA DESCIDA DO CURSOR
#define bconfirm   PORTAbits.RA2        // BOT�O DE CONFIRMA�AO PARA O DISPLAY
#define bretunr    PORTAbits.RA3        // BOT�O DE RETORNO PARA O DISPLAY
#define tempRef    28                   // TEMPERATURA DESEJADA
#define tempPin    PORTCbits.RC5

// DECLARA��O DE VARI�VEIS GLOBAIS______________________________________________
// VARI�VEIS RELACIONADO AO BOT�O DE EMERG�NCIA
unsigned char emergencia = 0;

// Hor�rio que iniciar� a alimenta��o 1x por dia
unsigned int  horas_alim = 9;   
unsigned int  min_alim   = 0;
unsigned int  sec_alim   = 0;

// Vari�veis relacionadas ao sistema de alimenta��o
unsigned int R1servo;   // Usado para controle do tempo em high do servo motor 1
unsigned int R2servo;   // Usado para controle do tempo em high do servo motor 2
unsigned int alimentacao = 0;   // 1 - horario de alimentar, 0 - ainda nao
unsigned int contadorTMR0;      // Verifica os PWM nos servos motores
float nvlAlimentacao;

// Vari�veis definidas para a extra��o do hor�rio do Real Time Clock
unsigned char timer[7];  // Vetor para guardar o hor�rio do RTC
unsigned int  ktimer = 0; // Vari�vel para a temporiza��o da extra��o do RTC

// Vari�veis para a interface homem-m�quina
unsigned int  ktemp   = 0;  // Vari�vel para a temporiza��o da extra��o da temper
unsigned char cursor = 2;  // Define a posi��o do cursor
/*
 * 1: Cursor na primeira Linha
 * 2: Cursor na segunda Linha
 * 3: Cursor na terceira Linha
 * 4: Cursor na quarta linha
 */
unsigned char tela   = 0; // Defini��o de qual tela est�
/*
 * 0: Tela Principal
 * 1: Tela de Informa��es
 * 2: Tela de Status
 * 3: Tela de Configura��o
 * 4: Tela de Configura��o de hor�rios
 */
unsigned char buffer_horaNova[3];       // Buffer de nova hora
unsigned char buffer_minutoNovo[3];     // Buffer de minuto novo
unsigned char ctela_config = 0;  
/*
 * 0: N�o entrou ainda na tela de configura��o de hor�rio (Tela 4)
 * 1: Entrou
 * 2: Config Horas
 * 3: Config Minutos
 */
unsigned char hora_nova;     // Usado como buffer para a mudan�a de hor�rio
unsigned char minuto_novo;   // Usado como buffer para a mudan�a de hor�rio
/*
 * 1: Tela inicial
 * 2: Config Horas
 * 3: Config Minutos
 */
unsigned char config_horario;
/*
 * 1: Configura��o do hor�rio da Ilumina��o
 * 2: Config do hor�rio da alimenta��o
 * 3: Config do hor�rio do RTC
 */
// PROT�TIPOS DAS SUBROTINAS____________________________________________________
void  inicializa_interrupcoes(void); 
void  horario_alimentacao(void);
void  readTimer(void);
float temperatura(void);
void  tempControl(void);
void  interfaceUser(void);

// SUBROTINA PARA INTERRUP��ES__________________________________________________
void __interrupt() intAll(void) {
    // INTERRUP�AO DO TIMER0: utilizado para o controle PWM dos servo motores
    if(TMR0IE && TMR0IF){ 
        TMR0IF  = 0; 
        if(!contadorTMR0){
            // Modula o PWM do primeiro servo-motor nos primeiros 10ms
            servo2 = 0;
            if(servo1){
                // mant�m n�vel l�gico baixo do servo1
                TMR0L   = 315 - R1servo; 
                servo1  = 0;
                contadorTMR0 = ~contadorTMR0;
            }else{
                // mant�m n�vel l�gico alto do servo1
                TMR0L   = R1servo;
                servo1  = 1;
            }
        } else {
            // Modula o PWM do segundo servo-motor nos 10ms finais
            servo1 = 0;
            if(servo2){
                // mant�m n�vel l�gico baixo do servo2
                TMR0L   = 315 - R2servo;
                servo2  = 0;
                contadorTMR0 = ~contadorTMR0;
            }else{
                // mant�m n�vel l�gico alto do servo2
                TMR0L   = R2servo;
                servo2  = 1;
            } 
        }
    }
    
    // INTERRUP��O DO CCP1: Utilizado para o echo no hcsr_1
    if(CCP1IF && CCP1IE) {
        if(CCP1CON == 0b00000101) {  
            // Habilita��o por borda de subida
            CCP1IE      = 0;            // Desabilita a interrup��o do CCP 1
            CCP1CON     = 0b00000100;   // Borda de captura para descida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
            CCP1IE      = 1;            // Habilita a interrup��o do CCP1
            TMR1H       = 0b00000000;   // Zeram os registradores do timer1
            TMR1L       = 0b00000000;   // Zeram os registradores do timer1
            TMR1ON      = 1;            // Habilita o timer1
        } else {
            // Habilita��o por borda de descida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
            CCP1IE      = 0;            // Desabilita a interrup��o do CCP 1
            TMR1ON      = 0;            // Desabilita o timer1
            tempo_h1    = CCPR1H;       // Carrengando os registradores de tempo
            tempo_l1    = CCPR1L;       // Carrengando os registradores de tempo
            CCP1CON     = 0b00000101;   // Borda de captura para subida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
        }
    }
    
    // INTERRUP��O DO CCP2: Utilizado para o echo no hcsr_2
    if(CCP2IF && CCP2IE) {
        if(CCP2CON == 0b00000101) {  
            // Habilita��o por borda de subida
            CCP2IE      = 0;            // Desabilita a interrup��o do CCP 1
            CCP2CON     = 0b00000100;   // Borda de capitura para descida
            CCP2IF      = 0;            // Limpa a flag para uma nova captura
            CCP2IE      = 1;            // Habilita a interrup��o do CCP1
            TMR3H       = 0x00;         // Zeram os registradores do timer1
            TMR3L       = 0x00;         // Zeram os registradores do timer1
            TMR3ON      = 1;            // Habilita o timer1
        } else {
            // Habilita��o por borda de descida
            tempo_h2    = CCPR2H;       // Carrengando os registradores de tempo
            tempo_l2    = CCPR2L;       // Carrengando os registradores de tempo
            TMR3ON      = 0;            // Desabilita o timer1
            CCP2IE      = 0;            // Desabilita a interrup��o do CCP 1
            CCP2CON     = 0b00000101;   // Borda de capitura para subida
            CCP2IF      = 0;            // Limpa a flag para uma nova captura
        }
    }
    
    // INTERRUP��O DO TIMER 2: Utilizado para a IHM
    if(TMR2IF && TMR2IE){
        ktimer++;                       // Adiciona ao contador
        if(ktimer > 19){
            readTimer();                // Leitura do hor�rio atual no RTC
            interfaceUser();            // Fun��o de interface homem-m�quina
            ktimer = 0;                 // Zera o contador
        }
        TMR2IF = 0;                     // Limpa a flag de interrup��o do Timer2
    }
    
    // INTERRUP��O DO INT0: Utilizado para habilitar o bot�o de emerg�ncia
    if(INT0E && INT0F) { 
        emergencia = 1;
        INTCON   = 0x00;        // Configura��es das Interrup��es
        INTCON2  = 0b11110101;  // Configura��es das Interrup��es
        T0CONbits.TMR0ON = 0;
        T1CONbits.TMR1ON = 0;
        T2CONbits.TMR2ON = 0;
        T3CONbits.TMR3ON = 0;
        INT0F = 0;      // Limpa a flag para uma nova captura de INT0
    }
}

// ROTINA PRINCIPAL_____________________________________________________________
void main(void) {
    // INICIALIZA��O DAS VARI�VEIS DO SISTEMA DE CONTROLE
    TRISCbits.RC5   = 0;   // RC5 configurado como sa�da
    
    // INICIALIZA��O DAS VARI�VEIS DO SISTEMA DE ILUMINA��O
    TRISBbits.RB7   = 0;   // RB7 configurado como sa�da 
    TRISBbits.RB6   = 0;   // RB6 configurado como sa�da
    contadorTMR0    = 0;   // Inicialmente como controle do primeiro servo-motor
    servo1          = 0;   // Pino do servo motor 1 como sa�da
    servo2          = 0;   // Pino do servo motor 2 como sa�da
    R1servo         = 227; // Inicialmente como 90� o �ngulo do primeiro servo
    R2servo         = 227; // Inicialmente como 90� o �ngulo do segundo servo
    alimentacao     = 0;   // Sinal de Alimenta��o ainda desativada
    
    // INICIALIZA��O DAS VARI�VEIS DA INTERFACE HOMEM-M�QUINA
    TRISAbits.RA0   = 1;                // RA0 configurado como entrada
    TRISAbits.RA1   = 1;                // RA1 configurado como entrada
    TRISAbits.RA2   = 1;                // RA2 configurado como entrada
    TRISAbits.RA3   = 1;                // RA3 configurado como entrada
    
    // INICIALIZA��O DAS VARI�VEIS DO SISTEMA DE EMERG�NCIA
    TRISBbits.RB0   = 1;               // RB0 configurado como entrada
    __delay_ms(1000);                  // Delay de 1 segundo
    
    // INICIALIZA��O DE SISTEMAS E DISPOSITIVOS
    Inicializa_LCD();                  // Configura o display LCD 20x4
    inicializa_hcsr04();               // Configura os sensores de ultrassom
    configurar_ilum();                 // Configura a ilumina��o
    iniciar_troca_agua();              // Configura o sist de troca de �gua
    inicializa_interrupcoes();         // Configura as interrup��es
    I2C_init(100000);                  // Inicializa a comunica��o I2C (100kHz)
    ds18b20_init(9);                   // Inicializa a comunica��o com o ds18b20
    
    // CONFIGURA��O INICIAL DO HOR�RIO
    Write_time(hora_atual, minuto_atual, segundo_atual, 0x01, 0x05, 0x09, 0x21);
    
    // LOOP INFINITO
    while(!emergencia) {
        tempControl();
        horario_alimentacao(); // Verifica se est� no hor�rio para a alimenta��o
        nFloatToStr(bufferNvlAgua, 1, nvlAgua);
        liga_lampada();        // Verifica se est� no hor�rio para a ilumina��o
        trocar_agua();         // Verifica se tem que trocar a �gua
    }
    
    // SITUA��O DE EMERG�NCIA
    while(1){
        __delay_ms(1000);
        servo1   =   0;         // PRIMEIRO SERVO MOTOR
        servo2   =   0;         // SEGUNDO SERVO MOTOR
        tempPin  =   0;
        ilum     =   0;
        trigger1 =   0;
        trigger2 =   0;
        valvula2 =   0;
        valvula1 =   0;
        Escreve_texto_com_pos(1,0,"     EMERGENCIA      ");
        Escreve_texto_com_pos(2,0,"     EMERGENCIA      ");
        Escreve_texto_com_pos(3,0,"     EMERGENCIA      ");
        Escreve_texto_com_pos(4,0,"     EMERGENCIA      ");
    }
    
    return;
}

// SUBROTINAS PARA A INICIALIZA��O DAS INTERRUP��ES_____________________________
void inicializa_interrupcoes(void) {
    INTCON  = 0b11110000;       // Configura��es das Interrup��es
    INTCON2 = 0b11100000;       // Configura��es das Interrup��es
    ADCON1 = 0x07;              // Configura todas as portas RA como digitais
    
    // CONFIGURA��O DA INTERRUP��O DO TIMER0 (Para controle de servo motores)
    T0CON   = 0b11000111;
    
    // CONFIGURA��O DA INTERRUP��O DO CCP1 E CCP2 (Sensores de ultrassom)
    IPEN        = 0;          // Prioridade de interrup��o desabilitado
    TMR1IE      = 0;          // desabilita a interrupcao do timer1
    TMR3IE      = 0;          // desabilita a interrupcao do timer3
    CCP1IE      = 0;          // Desabilita a interrup��o por captura (CPP1)
    CCP2IE      = 0;          // Desabilita a interrup��o por captura (CPP1)
    T1CON       = 0b10100000; // Configura��o do timer1
    T3CON       = 0b10101000; // Configura��o do timer3
    CCP2CON     = 0b00000101; // Configura a borda de captura para subida (CCP2)
    CCP1CON     = 0b00000101; // Configura a borda de captura para subida (CCP1)
    
    // CONFIGURA��O DA INTERRUP��O DO TIMER 2 (Amostragem da Temperatura)
    TMR2IE      = 1;            // Habilita��o do Timer2
    T2CON       = 0b01111110;   // Configura��o do timer2
    TMR2        = 0xFF;         // Configura a QTDE de estouros do timer2
    
    // CONFIGURA��O DA INTERRUP�AO DO INT0 (Para bot�o de emerg�ncia)
    // INTCON2bits.INTEDG0 = 1; 
    // INTCONbits.INT0F    = 0; 
    // INTCONbits.INT0E    = 1;
    // INTCONbits.GIE      = 1;
    
    ei();
}

// SUBROTINA PARA A VERIFICACAO E REALIZACAO DO SISTEMA DE ALIMENTACAO__________
void horario_alimentacao(void){
    /*
     *      Faz a medi��o do n�vel de ra��o e Abre os servos motores quando
     *      estiver no hor�rio solicitado para a alimenta��o
     */
    if(!sist_alim_on && !alimentacao){
        // Caso o sistema de alimenta��o esteja desativado
        return;
    }
    if(hora_atual == horas_alim && minuto_atual == min_alim
            && (segundo_atual >= sec_alim && segundo_atual <= sec_alim+5)) {
        distance_1();
        // nvlAlimentacao = (distancia_at�_base - distancia_medida)/distancia_medida_total
        // distanciaalimentacao varia de 3 at� 15 cm
        nvlAlimentacao = 100*(15 - distancia_1)/12;
        //nFloatToStr(bufferAlim, 2, distancia_1); // APAGAR DEPOIS
        nFloatToStr(bufferAlim, 0, nvlAlimentacao); 
        if(distancia_1 >= 13.5) {
            // Caso esteja vazio, adotando distancia de 13.5 cm
            //sprintf2(buffer2, "R.RACAO VAZIO   ");
        } else {
            //sprintf2(buffer2, "distancia: ");
            //sprintf2(buffer2, "distancia: ")
            //nFloatToStr(bufferAlim, 1, distancia_1);
            //nFloatToStr(buffer2+11, 1, distancia_1);
            alimentacao = 1;
        }
    } 
    else if (alimentacao == 1 && segundo_atual < sec_alim+12) {
        R1servo = 237;  // 90�
    } else if (alimentacao == 1 && segundo_atual < sec_alim+18) {
        R1servo = 227;  // 0�
    } else if (alimentacao == 1 && segundo_atual < sec_alim+23) {
        R2servo = 237;  // 90�
    } else {
        R2servo = 227;  // 0�
        alimentacao = 0;
        //if(segundo_atual == sec_alim+16)
        //    sprintf2(buffer2, "               ");
    }
}

// SUBROTINA PARA A EXTRA��O DO TIMER___________________________________________
void readTimer(){
    // Leitura do hor�rio do RTC
    Read_time(timer);               
    hora_atual    = timer[2];        
    minuto_atual  = timer[1];
    segundo_atual = timer[0];
    // Convers�o do hor�rio lido do RTC para um n�mero equivalente
    segundo_atual = 10*(((segundo_atual)&(0x70)) >> 4) + ((segundo_atual)&0x0F);
    minuto_atual  = 10*(((minuto_atual)&(0x70)) >> 4) + ((minuto_atual)&0x0F);       
    hora_atual    = 10*(((hora_atual)&(0x70)) >> 4) + ((hora_atual)&0x0F);
    // Guarda o hor�rio lido na vari�vel de hor�rio do display
    nFloatToStr(buffer+5, 0, hora_atual);
    nFloatToStr(buffer+8, 0, minuto_atual);
    nFloatToStr(buffer+11, 0, segundo_atual);
}

// SUBROTINA PARA A MEDI��O DA TEMPERATURA______________________________________
float temperatura(){
    int temp = ds18b20_readTemp();  // Leitura da temperatura da �gua
    float temperature;
    if(((temp & 0xF000)) == 0xF000)
        // Para a medi��o de temperatura abaixo de zero
        temperature = (((temp & 0x0FFF) >> 4) + (float) (temp & 0x0F)/16) - 256;
    else
        // Para medi��o de temperatura acima de zero
        temperature = (temp >> 4) + (float) (temp & 0x0F)/16;
    // Escreve a temperatura na sua vari�vel de buffer
    nFloatToStr(bufferTemp, 1, temperature);    
    return temperature;
}

// SUBROTINA PARA O CONTROLE DE TEMPERATURA DA �GUA_____________________________
void tempControl(){
    float temperature = temperatura();         // Medi��o da temperatura
    if(!sist_temp_on){
        // Caso o sistema de temperatura esteja desativado
        if(tempPin) tempPin = 0;
        return;
    }
    if(temperature < tempRef){
        /*
         * Caso a temperatura medida for menor que a temperatura de refer�ncia,
         * � ligado o pino da resist�ncia
         */
        tempPin = 1; 
    } else {
        /*
         * Caso a temperatura medida for maior ou igual a temperatura de 
         * refer�ncia, � desligado o pino da resist�ncia
         */
        tempPin = 0;
    }
}

// SUBROTINA PARA A INTERFACE COM O USU�RIO_____________________________________
void interfaceUser(){
    // SWITCH PARA A ESCOLHA DE TELA
    switch(tela){ 
        case 0:  
            // Tela Principal
            Escreve_texto_com_pos(1,1,buffer);
            Escreve_texto_com_pos(2,3,"INFORMACAO        ");
            Escreve_texto_com_pos(3,3,"STATUS            ");
            Escreve_texto_com_pos(4,3,"CONFIGURACAO      ");
            break;
        case 1:    
            // Tela de Informa��es para monitoramento
            Escreve_texto_com_pos(1,1,buffer);
            Escreve_texto_com_pos(2,3,"TEMPERATURA:");
            Escreve_texto_com_pos(3,3,"ALIMENTACAO:");
            Escreve_texto_com_pos(4,3,"NVL. AGUA:  ");
            Escreve_texto_com_pos(2,15, bufferTemp);
            Escreve_texto_com_pos(3,15, bufferAlim);
            Escreve_texto_com_pos(4,15, bufferNvlAgua);
            Escreve_texto_com_pos(2,20,"C");
            Escreve_texto_com_pos(3,20,"%");
            Escreve_texto_com_pos(4,20,"%");
            break;
        case 2:   
            // Tela de status dos sistemas
            Escreve_texto_com_pos(1,3,"SIST. DE TEMP: ");
            Escreve_texto_com_pos(2,3,"SIST. DE ILUM: ");
            Escreve_texto_com_pos(3,3,"SIST. DE ALIM: ");
            Escreve_texto_com_pos(4,3,"SIST.TROC AGUA:");
            // Verifica��o do estado ligado/desligado dos sistemas
            if(sist_temp_on)      Escreve_texto_com_pos(1, 18,"ON ");
            else                  Escreve_texto_com_pos(1, 18,"OFF");
            if(sist_ilum_on)      Escreve_texto_com_pos(2, 18,"ON ");
            else                  Escreve_texto_com_pos(2, 18,"OFF");
            if(sist_alim_on)      Escreve_texto_com_pos(3, 18,"ON ");
            else                  Escreve_texto_com_pos(3, 18,"OFF");
            if(sist_trocaAgua_on) Escreve_texto_com_pos(4, 18,"ON ");
            else                  Escreve_texto_com_pos(4, 18,"OFF");
            break;
        case 3:   
            // Tela de configura��o de hor�rios dos sistemas
            config_horario = 0;                        // Ainda nenhum escolhido
            Escreve_texto_com_pos(1,1,buffer);
            Escreve_texto_com_pos(2,3,"SIST. DE ILUM    ");
            Escreve_texto_com_pos(3,3,"SIST. DE ALIM    ");
            Escreve_texto_com_pos(4,3,"CONFIG HORARIO   ");
            break;
        case 4:
            // Tela de configura��o de hor�rio do sistema escolhido
            // Switch de escolha se ser� mudado as horas, ou oa minutos
            switch(ctela_config){
                case 0:
                    // Vai para o caso 1 (Retirar depois)
                    ctela_config = 1;
                case 1:
                    // Ainda nenhuma configura��o de tempo escolhida
                    Escreve_texto_com_pos(1,3,bufferConfig);
                    Escreve_texto_com_pos(2,3,"Horas:       ");
                    Escreve_texto_com_pos(3,3,"Minutos:     ");
                    Escreve_texto_com_pos(4,3,"    Confirmar    ");
                    Escreve_texto_com_pos(2, 17, buffer_horaNova);
                    Escreve_texto_com_pos(3, 17, buffer_minutoNovo);
                    break;
                case 2:
                    // Configura��o de horas de algum sistema
                    Escreve_texto_com_pos(1,3,bufferConfig);
                    Escreve_texto_com_pos(2,3,"Horas:       ");
                    Escreve_texto_com_pos(3,3,"                 ");
                    Escreve_texto_com_pos(4,3,"    Confirmar    ");
                    Escreve_texto_com_pos(2, 17, buffer_horaNova);
                    break;
                case 3:
                    // Configura��o de minutos de algum sistema
                    Escreve_texto_com_pos(1,3,bufferConfig);
                    Escreve_texto_com_pos(2,3,"                 ");
                    Escreve_texto_com_pos(3,3,"Minutos:     ");
                    Escreve_texto_com_pos(4,3,"    Confirmar    ");
                    Escreve_texto_com_pos(3, 17, buffer_minutoNovo);
                    break;
                default:
                    break;
            }
            break;
        default:
            Escreve_texto_com_pos(1,1,buffer);
            Escreve_texto_com_pos(2,3,"INFORMACAO        ");
            Escreve_texto_com_pos(3,3,"STATUS            ");
            Escreve_texto_com_pos(4,3,"CONFIGURACAO      ");
            break;
    }
    
    //CONFIGURACOES DOS BOTOES 
    if (bup){ 
        // BOT�O UP ?
        if(ctela_config <= 1){
        // Para mudar o cursor quando n�o estiver nas configura��es de hor�rio
            Escreve_texto_com_pos(cursor,1," ");          
            cursor--;                        // Coloca o cursor na linha de cima                      
            if(cursor == 1 && tela != 2) cursor = 4;
            /*
             * Caso o cursor esteja na segunda linha numa tela que n�o seja a de
             * 'Status', ele volta para a ultima linha
             */
            else if(cursor == 0 && tela == 2) cursor = 4;
            /*
             * Caso o cursor esteja na primeira linha numa tela que seja a de
             * 'Status', ele volta para a ultima linha
             */
        } else{
        // Para mudar o hor�rio quando � precionado as setas
            if(ctela_config == 2){ 
                // Configura��o de horas
                hora_nova = (hora_nova+1)%24;     // Adiciona +1 para as horas
                nFloatToStr(buffer_horaNova, 0, hora_nova);
            } else{                
                // Configura��o dos minutos
                minuto_novo = (minuto_novo+1)%60; // Adiciona +1 para os minutos
                nFloatToStr(buffer_minutoNovo, 0, minuto_novo);
            }
        }
    } else if (bdown) {
        // BOT�O DOWN ?
        if(ctela_config <= 1){
        // Para mudar o cursor quando n�o estiver nas configura��es de hor�rio
            Escreve_texto_com_pos(cursor,1," ");      // Apaga o cursor
            // Coloca o cursor na linha de baixo
            cursor = cursor%4 + 1;          
            if(cursor == 1 && tela != 2) cursor = 2;
        } else {
            // Caso esteja no modo de configura��o de hor�rio
            if(ctela_config == 2){
                // Configura��o de horas
                if(hora_nova > 0)           hora_nova--;
                else                        hora_nova = 23;
                nFloatToStr(buffer_horaNova, 0, hora_nova);
            } else{
                // Configura��o dos minutos
                if(minuto_novo > 0)           minuto_novo--;
                else                          minuto_novo = 59;
                nFloatToStr(buffer_minutoNovo, 0, minuto_novo);
            }
        }
    } else if(bconfirm){
        // BOT�O DE CONFIRMA��O
        switch(tela){
            // switch para a verifica��o de tela e posi��o do cursor
            case 0:
                // Caso esteja na tela principal
                switch(cursor){
                    // switch para a verifica��o da posi��o do cursor
                    case 2:
                        // Mudan�a para a tela de informa��es
                        Escreve_texto_com_pos(cursor,1," ");
                        cursor = 2;
                        tela = 1;
                        break;
                    case 3:
                        // Mudan�a para a tela de status
                        Escreve_texto_com_pos(cursor,1," ");
                        cursor = 1;
                        tela = 2;
                        break;
                    case 4:
                        // Mudan�a para a tela de configura��es
                        Escreve_texto_com_pos(cursor,1," ");
                        cursor = 2;
                        tela = 3;
                        break;
                }
                break;
            case 1:
                // Caso esteja na tela de informa��es
                break;
            case 2:
                // Caso esteja na tela de Status
                switch(cursor){
                    // switch para a verifica��o da posi��o do cursor
                    case 1:
                        // Liga/Desliga o sistema de temperatura
                        sist_temp_on      = !sist_temp_on;
                        break;
                    case 2:
                        // Liga/Desliga o sistema de ilumina��o
                        sist_ilum_on      = !sist_ilum_on;
                        break;
                    case 3:
                        // Liga/Desliga o sistema de alimenta��o
                        sist_alim_on      = !sist_alim_on;
                        break;
                    case 4:
                        // Liga/Desliga o sistema de troca de �gua
                        sist_trocaAgua_on = !sist_trocaAgua_on;
                        break;
                    default:
                        break;
                }
                break;
            case 3:
                // Caso esteja na tela de Configura��o
                tela = 4; // Vai para a tela de Configura��o de hor�rio
                switch(cursor){
                    // switch para a verifica��o da posi��o do cursor
                    case 2:
                        // Vai para a tela de configura��o do sit. de ilumina��o
                        config_horario = 1; 
                        sprintf2(bufferConfig, "SIST. DE ILUM");
                        nFloatToStr(buffer_horaNova, 0, hour_ilum);
                        nFloatToStr(buffer_minutoNovo, 0, min_ilum);
                        hora_nova   = hour_ilum;
                        minuto_novo = min_ilum;
                        break;
                    case 3:
                        // Vai para a tela de configura��o do sit. alimenta��o
                        config_horario = 2; 
                        sprintf2(bufferConfig, "SIST. DE ALIM");
                        nFloatToStr(buffer_horaNova, 0, horas_alim);
                        nFloatToStr(buffer_minutoNovo, 0, min_alim);
                        hora_nova   = horas_alim;
                        minuto_novo = min_alim;
                        break;
                    case 4:
                        // Vai para a tela de configura��o do hor�rio do RTC
                        config_horario = 3; 
                        sprintf2(bufferConfig, "CONFIG. HORARIO");
                        nFloatToStr(buffer_horaNova, 0, hora_atual);
                        nFloatToStr(buffer_minutoNovo, 0, minuto_atual);
                        hora_nova   = hora_atual;
                        minuto_novo = minuto_atual;
                        break;
                    default:
                        break;
                }
                break;
            case 4:    
                // Caso esteja na tela de Configura��o de algum sistema
                switch(cursor){
                    // switch para a verifica��o da posi��o do cursor
                    case 2:
                        // Seleciona as horas para a configura��o
                        ctela_config = 2;
                        break;
                    case 3:
                        // Seleciona os minutos para a configura��o
                        ctela_config = 3;
                        break;
                    case 4:
                        // Realiza a confirma��o do hor�rio modificado
                        switch(config_horario){
                        // switch para a verifica��o de qual sistema deve mudar
                            case 1:
                                // Mudan�a de hor�rio do sistema de ilumina��o
                                hour_ilum = hora_nova;
                                min_ilum  = minuto_novo;
                                tela = 3;
                                break;
                            case 2:
                                // Mudan�a de hor�rio do sistema de alimenta��o
                                horas_alim = hora_nova;
                                min_alim   = minuto_novo;
                                tela = 3;
                                break;
                            case 3:
                                // Mudan�a de hor�rio do RTC
                                hora_nova   =  (hora_nova%10) | ((hora_nova/10) << 4);
                                minuto_novo =  (minuto_novo%10) | ((minuto_novo/10) << 4);     
                                Write_time(hora_nova, minuto_novo, 0x00, 0x01, 0x05, 0x09, 0x21);
                                tela = 3;
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    } else if(bretunr){// BOT�O DE RETORNO
        
        // Retorno para a tela principal se estiver na tela 1, 2 ou 3
        if(tela == 1 || tela == 2 || tela == 3) tela = 0;
        // Retorna para a tela de Configura��o se estiver na tela 4
        if(tela == 4) {
            if(ctela_config <= 1){
                tela = 3;
            } else {
                
            }
            ctela_config = 1;
        }
        Escreve_texto_com_pos(cursor,1," ");
        cursor = 2;                          // Volta o cursor para a 2� posi��o
    }
    Escreve_texto_com_pos(cursor,1,">");     // Escreve a seta do cursor
}