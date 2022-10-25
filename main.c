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
 *      (a) Sistema de Temperatura                (OK)
 *      (b) Sistema de Alimentação                (OK)
 *      (c) Sistema de Iluminação UVB + UVA       (OK)
 *      (d) Sistema de Troca de Água              (OK)
 * 
 * fclk  = 20 MHz
 * 
 * Conexões:
 *      1  - MCLR_/VPP:  Botão de Reset
 *      2  - RA0:        Botão IHM "cursor para cima"  (Entrada)
 *      3  - RA1:        Botão IHM "cursor para baixo" (Entrada)
 *      4  - RA2:        Botão IHM "confirmar"         (Entrada)
 *      5  - RA3:        Botão IHM "sair"              (Entrada)
 *      6  - RA4:        Conexão OneWire com o DS18B20 (Entrada/Saída)
 *      13 - OSC1_/CLK1: Primeira conexão com o circuito oscilador
 *      14 - OSC2_/CLK2: Segunda conexão com o circuito oscilador
 *      15 - RC0:        Trigger do HCSR04 do sistema de alimentação (Saída)
 *      16 - RC1:        Echo do HCSR04 do sistema de troca de água  (Entrada)
 *      17 - RC2:        Echo do HCSR04 do sistema de alimentação    (Saída)
 *      18 - RC3/SCL:    Conexão SCL da comunicação I²C com o DS1307 (Saída)
 *      19 - RD0:        Trigger do HCSR04 do sistema de troca de água (Saída)
 *      20 - RD1:        Conexão com a lâmpada UVA/UVB (Saída)
 *      21 - RD2:        Conexão RS do display LCD 20x4  (Saída)
 *      22 - RD3:        Conexão E do display LCD 20x4   (Saída)
 *      23 - RC4/SDA:    Conexão SDA da comunicação I²C com o DS1307 (E/S) 
 *      24 - RC5:        Conexão com a resistência de aquecimento de água (S)
 *      25 - RC6:        Conexão com a válvula solenóide de saída de água (S)
 *      26 - RC7:        Conexão com a válvula solenóide de entrada de água (S)
 *      27 - RD4:        Conexão D4 do display LCD 20x4  (Saída)
 *      28 - RD5:        Conexão D5 do display LCD 20x4  (Saída)
 *      29 - RD6:        Conexão D6 do display LCD 20x4  (Saída)
 *      30 - RD7:        Conexão D7 do display LCD 20x4  (Saída)
 *      33 - RB0/INT0:   Botão de emergência (Entrada)
 *      34 - RB1:        Botão para a entrada de água  (Entrada)
 *      35 - RB2:        Botão para a saída de água    (Entrada)
 *      
 *      39 - RB6:        Conexão PWM com o segundo servo-motor  (Saída)
 *      40 - RB7:        Conexão PWM com o primeiro servo-motor (Saída)
 *      
 */

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS______________________________________________
char buffer[21]       = "       :  :         ";     // Primeira linha LCD
char bufferNvlAgua[5] = "    ";                     // Buffer para nvl da águal
char bufferAlim[5]    = "    ";                     // Buffer para alimentação
char bufferTemp[6]    = "     ";                    // Buffer para temperatura
char bufferConfig[21]     = "                    "; // Buffer para configuração

// Horário definido para a configuração inicial
unsigned int hora_atual    = 0x08;                  // Hora atual
unsigned int minuto_atual  = 0x59;                  // Minuto atuall
unsigned int segundo_atual = 0x55;                  // Segundo atual

unsigned char sist_temp_on      = 1;    // Sistema de temperatura   ON/OFF
unsigned char sist_ilum_on      = 1;    // Sistema de iluminaçao    ON/OFF
unsigned char sist_alim_on      = 1;    // Sistema de alimentação   ON/OFF
unsigned char sist_trocaAgua_on = 1;    // Sistema de troca de água ON/OFF

// INCLUSÃO DE BIBLIOTECAS______________________________________________________
#include "string2.h"
#include "18F452_config_bits.h"
#include "ds18b20.h"
#include "LCD_20x4.h"
#include "iluminacao.h"
#include "hcsr04.h"
#include "troca_agua_2.h"
#include "ds1307.h"

// DEFINIÇÕES___________________________________________________________________
#define emerg      PORTBbits.RB0        // BOTÂO DE EMERGÊNCIA
#define servo1     PORTBbits.RB7        // PRIMEIRO SERVO MOTOR
#define servo2     PORTBbits.RB6        // SEGUNDO SERVO MOTOR
#define bup        PORTAbits.RA0        // BOTÃO PARA SUBIDA DO CURSOR
#define bdown      PORTAbits.RA1        // BOTÃO PARA DESCIDA DO CURSOR
#define bconfirm   PORTAbits.RA2        // BOTÃO DE CONFIRMAÇAO PARA O DISPLAY
#define bretunr    PORTAbits.RA3        // BOTÃO DE RETORNO PARA O DISPLAY
#define tempRef    28                   // TEMPERATURA DESEJADA
#define tempPin    PORTCbits.RC5

// DECLARAÇÃO DE VARIÁVEIS GLOBAIS______________________________________________
// VARIÁVEIS RELACIONADO AO BOTÃO DE EMERGÊNCIA
unsigned char emergencia = 0;

// Horário que iniciará a alimentação 1x por dia
unsigned int  horas_alim = 9;   
unsigned int  min_alim   = 0;
unsigned int  sec_alim   = 0;

// Variáveis relacionadas ao sistema de alimentação
unsigned int R1servo;   // Usado para controle do tempo em high do servo motor 1
unsigned int R2servo;   // Usado para controle do tempo em high do servo motor 2
unsigned int alimentacao = 0;   // 1 - horario de alimentar, 0 - ainda nao
unsigned int contadorTMR0;      // Verifica os PWM nos servos motores
float nvlAlimentacao;

// Variáveis definidas para a extração do horário do Real Time Clock
unsigned char timer[7];  // Vetor para guardar o horário do RTC
unsigned int  ktimer = 0; // Variável para a temporização da extração do RTC

// Variáveis para a interface homem-máquina
unsigned int  ktemp   = 0;  // Variável para a temporização da extração da temper
unsigned char cursor = 2;  // Define a posição do cursor
/*
 * 1: Cursor na primeira Linha
 * 2: Cursor na segunda Linha
 * 3: Cursor na terceira Linha
 * 4: Cursor na quarta linha
 */
unsigned char tela   = 0; // Definição de qual tela está
/*
 * 0: Tela Principal
 * 1: Tela de Informações
 * 2: Tela de Status
 * 3: Tela de Configuração
 * 4: Tela de Configuração de horários
 */
unsigned char buffer_horaNova[3];       // Buffer de nova hora
unsigned char buffer_minutoNovo[3];     // Buffer de minuto novo
unsigned char ctela_config = 0;  
/*
 * 0: Não entrou ainda na tela de configuração de horário (Tela 4)
 * 1: Entrou
 * 2: Config Horas
 * 3: Config Minutos
 */
unsigned char hora_nova;     // Usado como buffer para a mudança de horário
unsigned char minuto_novo;   // Usado como buffer para a mudança de horário
/*
 * 1: Tela inicial
 * 2: Config Horas
 * 3: Config Minutos
 */
unsigned char config_horario;
/*
 * 1: Configuração do horário da Iluminação
 * 2: Config do horário da alimentação
 * 3: Config do horário do RTC
 */
// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void  inicializa_interrupcoes(void); 
void  horario_alimentacao(void);
void  readTimer(void);
float temperatura(void);
void  tempControl(void);
void  interfaceUser(void);

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
            CCP1IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP1CON     = 0b00000100;   // Borda de captura para descida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
            CCP1IE      = 1;            // Habilita a interrupção do CCP1
            TMR1H       = 0b00000000;   // Zeram os registradores do timer1
            TMR1L       = 0b00000000;   // Zeram os registradores do timer1
            TMR1ON      = 1;            // Habilita o timer1
        } else {
            // Habilitação por borda de descida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
            CCP1IE      = 0;            // Desabilita a interrupção do CCP 1
            TMR1ON      = 0;            // Desabilita o timer1
            tempo_h1    = CCPR1H;       // Carrengando os registradores de tempo
            tempo_l1    = CCPR1L;       // Carrengando os registradores de tempo
            CCP1CON     = 0b00000101;   // Borda de captura para subida
            CCP1IF      = 0;            // Limpa a flag para uma nova captura
        }
    }
    
    // INTERRUPÇÃO DO CCP2: Utilizado para o echo no hcsr_2
    if(CCP2IF && CCP2IE) {
        if(CCP2CON == 0b00000101) {  
            // Habilitação por borda de subida
            CCP2IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP2CON     = 0b00000100;   // Borda de capitura para descida
            CCP2IF      = 0;            // Limpa a flag para uma nova captura
            CCP2IE      = 1;            // Habilita a interrupção do CCP1
            TMR3H       = 0x00;         // Zeram os registradores do timer1
            TMR3L       = 0x00;         // Zeram os registradores do timer1
            TMR3ON      = 1;            // Habilita o timer1
        } else {
            // Habilitação por borda de descida
            tempo_h2    = CCPR2H;       // Carrengando os registradores de tempo
            tempo_l2    = CCPR2L;       // Carrengando os registradores de tempo
            TMR3ON      = 0;            // Desabilita o timer1
            CCP2IE      = 0;            // Desabilita a interrupção do CCP 1
            CCP2CON     = 0b00000101;   // Borda de capitura para subida
            CCP2IF      = 0;            // Limpa a flag para uma nova captura
        }
    }
    
    // INTERRUPÇÃO DO TIMER 2: Utilizado para a IHM
    if(TMR2IF && TMR2IE){
        ktimer++;                       // Adiciona ao contador
        if(ktimer > 19){
            readTimer();                // Leitura do horário atual no RTC
            interfaceUser();            // Função de interface homem-máquina
            ktimer = 0;                 // Zera o contador
        }
        TMR2IF = 0;                     // Limpa a flag de interrupção do Timer2
    }
    
    // INTERRUPÇÃO DO INT0: Utilizado para habilitar o botão de emergência
    if(INT0E && INT0F) { 
        emergencia = 1;
        INTCON   = 0x00;        // Configurações das Interrupções
        INTCON2  = 0b11110101;  // Configurações das Interrupções
        T0CONbits.TMR0ON = 0;
        T1CONbits.TMR1ON = 0;
        T2CONbits.TMR2ON = 0;
        T3CONbits.TMR3ON = 0;
        INT0F = 0;      // Limpa a flag para uma nova captura de INT0
    }
}

// ROTINA PRINCIPAL_____________________________________________________________
void main(void) {
    // INICIALIZAÇÃO DAS VARIÁVEIS DO SISTEMA DE CONTROLE
    TRISCbits.RC5   = 0;   // RC5 configurado como saída
    
    // INICIALIZAÇÃO DAS VARIÁVEIS DO SISTEMA DE ILUMINAÇÃO
    TRISBbits.RB7   = 0;   // RB7 configurado como saída 
    TRISBbits.RB6   = 0;   // RB6 configurado como saída
    contadorTMR0    = 0;   // Inicialmente como controle do primeiro servo-motor
    servo1          = 0;   // Pino do servo motor 1 como saída
    servo2          = 0;   // Pino do servo motor 2 como saída
    R1servo         = 227; // Inicialmente como 90° o ângulo do primeiro servo
    R2servo         = 227; // Inicialmente como 90° o ângulo do segundo servo
    alimentacao     = 0;   // Sinal de Alimentação ainda desativada
    
    // INICIALIZAÇÃO DAS VARIÁVEIS DA INTERFACE HOMEM-MÁQUINA
    TRISAbits.RA0   = 1;                // RA0 configurado como entrada
    TRISAbits.RA1   = 1;                // RA1 configurado como entrada
    TRISAbits.RA2   = 1;                // RA2 configurado como entrada
    TRISAbits.RA3   = 1;                // RA3 configurado como entrada
    
    // INICIALIZAÇÃO DAS VARIÁVEIS DO SISTEMA DE EMERGÊNCIA
    TRISBbits.RB0   = 1;               // RB0 configurado como entrada
    __delay_ms(1000);                  // Delay de 1 segundo
    
    // INICIALIZAÇÃO DE SISTEMAS E DISPOSITIVOS
    Inicializa_LCD();                  // Configura o display LCD 20x4
    inicializa_hcsr04();               // Configura os sensores de ultrassom
    configurar_ilum();                 // Configura a iluminação
    iniciar_troca_agua();              // Configura o sist de troca de água
    inicializa_interrupcoes();         // Configura as interrupções
    I2C_init(100000);                  // Inicializa a comunicação I2C (100kHz)
    ds18b20_init(9);                   // Inicializa a comunicação com o ds18b20
    
    // CONFIGURAÇÃO INICIAL DO HORÁRIO
    Write_time(hora_atual, minuto_atual, segundo_atual, 0x01, 0x05, 0x09, 0x21);
    
    // LOOP INFINITO
    while(!emergencia) {
        tempControl();
        horario_alimentacao(); // Verifica se está no horário para a alimentação
        nFloatToStr(bufferNvlAgua, 1, nvlAgua);
        liga_lampada();        // Verifica se está no horário para a iluminação
        trocar_agua();         // Verifica se tem que trocar a água
    }
    
    // SITUAÇÃO DE EMERGÊNCIA
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

// SUBROTINAS PARA A INICIALIZAÇÃO DAS INTERRUPÇÕES_____________________________
void inicializa_interrupcoes(void) {
    INTCON  = 0b11110000;       // Configurações das Interrupções
    INTCON2 = 0b11100000;       // Configurações das Interrupções
    ADCON1 = 0x07;              // Configura todas as portas RA como digitais
    
    // CONFIGURAÇÃO DA INTERRUPÇÃO DO TIMER0 (Para controle de servo motores)
    T0CON   = 0b11000111;
    
    // CONFIGURAÇÃO DA INTERRUPÇÃO DO CCP1 E CCP2 (Sensores de ultrassom)
    IPEN        = 0;          // Prioridade de interrupção desabilitado
    TMR1IE      = 0;          // desabilita a interrupcao do timer1
    TMR3IE      = 0;          // desabilita a interrupcao do timer3
    CCP1IE      = 0;          // Desabilita a interrupção por captura (CPP1)
    CCP2IE      = 0;          // Desabilita a interrupção por captura (CPP1)
    T1CON       = 0b10100000; // Configuração do timer1
    T3CON       = 0b10101000; // Configuração do timer3
    CCP2CON     = 0b00000101; // Configura a borda de captura para subida (CCP2)
    CCP1CON     = 0b00000101; // Configura a borda de captura para subida (CCP1)
    
    // CONFIGURAÇÃO DA INTERRUPÇÃO DO TIMER 2 (Amostragem da Temperatura)
    TMR2IE      = 1;            // Habilitação do Timer2
    T2CON       = 0b01111110;   // Configuração do timer2
    TMR2        = 0xFF;         // Configura a QTDE de estouros do timer2
    
    // CONFIGURAÇÃO DA INTERRUPÇAO DO INT0 (Para botão de emergência)
    // INTCON2bits.INTEDG0 = 1; 
    // INTCONbits.INT0F    = 0; 
    // INTCONbits.INT0E    = 1;
    // INTCONbits.GIE      = 1;
    
    ei();
}

// SUBROTINA PARA A VERIFICACAO E REALIZACAO DO SISTEMA DE ALIMENTACAO__________
void horario_alimentacao(void){
    /*
     *      Faz a medição do nível de ração e Abre os servos motores quando
     *      estiver no horário solicitado para a alimentação
     */
    if(!sist_alim_on && !alimentacao){
        // Caso o sistema de alimentação esteja desativado
        return;
    }
    if(hora_atual == horas_alim && minuto_atual == min_alim
            && (segundo_atual >= sec_alim && segundo_atual <= sec_alim+5)) {
        distance_1();
        // nvlAlimentacao = (distancia_até_base - distancia_medida)/distancia_medida_total
        // distanciaalimentacao varia de 3 até 15 cm
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
        R1servo = 237;  // 90°
    } else if (alimentacao == 1 && segundo_atual < sec_alim+18) {
        R1servo = 227;  // 0°
    } else if (alimentacao == 1 && segundo_atual < sec_alim+23) {
        R2servo = 237;  // 90º
    } else {
        R2servo = 227;  // 0°
        alimentacao = 0;
        //if(segundo_atual == sec_alim+16)
        //    sprintf2(buffer2, "               ");
    }
}

// SUBROTINA PARA A EXTRAÇÃO DO TIMER___________________________________________
void readTimer(){
    // Leitura do horário do RTC
    Read_time(timer);               
    hora_atual    = timer[2];        
    minuto_atual  = timer[1];
    segundo_atual = timer[0];
    // Conversão do horário lido do RTC para um número equivalente
    segundo_atual = 10*(((segundo_atual)&(0x70)) >> 4) + ((segundo_atual)&0x0F);
    minuto_atual  = 10*(((minuto_atual)&(0x70)) >> 4) + ((minuto_atual)&0x0F);       
    hora_atual    = 10*(((hora_atual)&(0x70)) >> 4) + ((hora_atual)&0x0F);
    // Guarda o horário lido na variável de horário do display
    nFloatToStr(buffer+5, 0, hora_atual);
    nFloatToStr(buffer+8, 0, minuto_atual);
    nFloatToStr(buffer+11, 0, segundo_atual);
}

// SUBROTINA PARA A MEDIÇÃO DA TEMPERATURA______________________________________
float temperatura(){
    int temp = ds18b20_readTemp();  // Leitura da temperatura da água
    float temperature;
    if(((temp & 0xF000)) == 0xF000)
        // Para a medição de temperatura abaixo de zero
        temperature = (((temp & 0x0FFF) >> 4) + (float) (temp & 0x0F)/16) - 256;
    else
        // Para medição de temperatura acima de zero
        temperature = (temp >> 4) + (float) (temp & 0x0F)/16;
    // Escreve a temperatura na sua variável de buffer
    nFloatToStr(bufferTemp, 1, temperature);    
    return temperature;
}

// SUBROTINA PARA O CONTROLE DE TEMPERATURA DA ÁGUA_____________________________
void tempControl(){
    float temperature = temperatura();         // Medição da temperatura
    if(!sist_temp_on){
        // Caso o sistema de temperatura esteja desativado
        if(tempPin) tempPin = 0;
        return;
    }
    if(temperature < tempRef){
        /*
         * Caso a temperatura medida for menor que a temperatura de referência,
         * é ligado o pino da resistência
         */
        tempPin = 1; 
    } else {
        /*
         * Caso a temperatura medida for maior ou igual a temperatura de 
         * referência, é desligado o pino da resistência
         */
        tempPin = 0;
    }
}

// SUBROTINA PARA A INTERFACE COM O USUÁRIO_____________________________________
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
            // Tela de Informações para monitoramento
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
            // Verificação do estado ligado/desligado dos sistemas
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
            // Tela de configuração de horários dos sistemas
            config_horario = 0;                        // Ainda nenhum escolhido
            Escreve_texto_com_pos(1,1,buffer);
            Escreve_texto_com_pos(2,3,"SIST. DE ILUM    ");
            Escreve_texto_com_pos(3,3,"SIST. DE ALIM    ");
            Escreve_texto_com_pos(4,3,"CONFIG HORARIO   ");
            break;
        case 4:
            // Tela de configuração de horário do sistema escolhido
            // Switch de escolha se será mudado as horas, ou oa minutos
            switch(ctela_config){
                case 0:
                    // Vai para o caso 1 (Retirar depois)
                    ctela_config = 1;
                case 1:
                    // Ainda nenhuma configuração de tempo escolhida
                    Escreve_texto_com_pos(1,3,bufferConfig);
                    Escreve_texto_com_pos(2,3,"Horas:       ");
                    Escreve_texto_com_pos(3,3,"Minutos:     ");
                    Escreve_texto_com_pos(4,3,"    Confirmar    ");
                    Escreve_texto_com_pos(2, 17, buffer_horaNova);
                    Escreve_texto_com_pos(3, 17, buffer_minutoNovo);
                    break;
                case 2:
                    // Configuração de horas de algum sistema
                    Escreve_texto_com_pos(1,3,bufferConfig);
                    Escreve_texto_com_pos(2,3,"Horas:       ");
                    Escreve_texto_com_pos(3,3,"                 ");
                    Escreve_texto_com_pos(4,3,"    Confirmar    ");
                    Escreve_texto_com_pos(2, 17, buffer_horaNova);
                    break;
                case 3:
                    // Configuração de minutos de algum sistema
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
        // BOTÂO UP ?
        if(ctela_config <= 1){
        // Para mudar o cursor quando não estiver nas configurações de horário
            Escreve_texto_com_pos(cursor,1," ");          
            cursor--;                        // Coloca o cursor na linha de cima                      
            if(cursor == 1 && tela != 2) cursor = 4;
            /*
             * Caso o cursor esteja na segunda linha numa tela que não seja a de
             * 'Status', ele volta para a ultima linha
             */
            else if(cursor == 0 && tela == 2) cursor = 4;
            /*
             * Caso o cursor esteja na primeira linha numa tela que seja a de
             * 'Status', ele volta para a ultima linha
             */
        } else{
        // Para mudar o horário quando é precionado as setas
            if(ctela_config == 2){ 
                // Configuração de horas
                hora_nova = (hora_nova+1)%24;     // Adiciona +1 para as horas
                nFloatToStr(buffer_horaNova, 0, hora_nova);
            } else{                
                // Configuração dos minutos
                minuto_novo = (minuto_novo+1)%60; // Adiciona +1 para os minutos
                nFloatToStr(buffer_minutoNovo, 0, minuto_novo);
            }
        }
    } else if (bdown) {
        // BOTÂO DOWN ?
        if(ctela_config <= 1){
        // Para mudar o cursor quando não estiver nas configurações de horário
            Escreve_texto_com_pos(cursor,1," ");      // Apaga o cursor
            // Coloca o cursor na linha de baixo
            cursor = cursor%4 + 1;          
            if(cursor == 1 && tela != 2) cursor = 2;
        } else {
            // Caso esteja no modo de configuração de horário
            if(ctela_config == 2){
                // Configuração de horas
                if(hora_nova > 0)           hora_nova--;
                else                        hora_nova = 23;
                nFloatToStr(buffer_horaNova, 0, hora_nova);
            } else{
                // Configuração dos minutos
                if(minuto_novo > 0)           minuto_novo--;
                else                          minuto_novo = 59;
                nFloatToStr(buffer_minutoNovo, 0, minuto_novo);
            }
        }
    } else if(bconfirm){
        // BOTÂO DE CONFIRMAÇÃO
        switch(tela){
            // switch para a verificação de tela e posição do cursor
            case 0:
                // Caso esteja na tela principal
                switch(cursor){
                    // switch para a verificação da posição do cursor
                    case 2:
                        // Mudança para a tela de informações
                        Escreve_texto_com_pos(cursor,1," ");
                        cursor = 2;
                        tela = 1;
                        break;
                    case 3:
                        // Mudança para a tela de status
                        Escreve_texto_com_pos(cursor,1," ");
                        cursor = 1;
                        tela = 2;
                        break;
                    case 4:
                        // Mudança para a tela de configurações
                        Escreve_texto_com_pos(cursor,1," ");
                        cursor = 2;
                        tela = 3;
                        break;
                }
                break;
            case 1:
                // Caso esteja na tela de informações
                break;
            case 2:
                // Caso esteja na tela de Status
                switch(cursor){
                    // switch para a verificação da posição do cursor
                    case 1:
                        // Liga/Desliga o sistema de temperatura
                        sist_temp_on      = !sist_temp_on;
                        break;
                    case 2:
                        // Liga/Desliga o sistema de iluminação
                        sist_ilum_on      = !sist_ilum_on;
                        break;
                    case 3:
                        // Liga/Desliga o sistema de alimentação
                        sist_alim_on      = !sist_alim_on;
                        break;
                    case 4:
                        // Liga/Desliga o sistema de troca de água
                        sist_trocaAgua_on = !sist_trocaAgua_on;
                        break;
                    default:
                        break;
                }
                break;
            case 3:
                // Caso esteja na tela de Configuração
                tela = 4; // Vai para a tela de Configuração de horário
                switch(cursor){
                    // switch para a verificação da posição do cursor
                    case 2:
                        // Vai para a tela de configuração do sit. de iluminação
                        config_horario = 1; 
                        sprintf2(bufferConfig, "SIST. DE ILUM");
                        nFloatToStr(buffer_horaNova, 0, hour_ilum);
                        nFloatToStr(buffer_minutoNovo, 0, min_ilum);
                        hora_nova   = hour_ilum;
                        minuto_novo = min_ilum;
                        break;
                    case 3:
                        // Vai para a tela de configuração do sit. alimentação
                        config_horario = 2; 
                        sprintf2(bufferConfig, "SIST. DE ALIM");
                        nFloatToStr(buffer_horaNova, 0, horas_alim);
                        nFloatToStr(buffer_minutoNovo, 0, min_alim);
                        hora_nova   = horas_alim;
                        minuto_novo = min_alim;
                        break;
                    case 4:
                        // Vai para a tela de configuração do horário do RTC
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
                // Caso esteja na tela de Configuração de algum sistema
                switch(cursor){
                    // switch para a verificação da posição do cursor
                    case 2:
                        // Seleciona as horas para a configuração
                        ctela_config = 2;
                        break;
                    case 3:
                        // Seleciona os minutos para a configuração
                        ctela_config = 3;
                        break;
                    case 4:
                        // Realiza a confirmação do horário modificado
                        switch(config_horario){
                        // switch para a verificação de qual sistema deve mudar
                            case 1:
                                // Mudança de horário do sistema de iluminação
                                hour_ilum = hora_nova;
                                min_ilum  = minuto_novo;
                                tela = 3;
                                break;
                            case 2:
                                // Mudança de horário do sistema de alimentação
                                horas_alim = hora_nova;
                                min_alim   = minuto_novo;
                                tela = 3;
                                break;
                            case 3:
                                // Mudança de horário do RTC
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
    } else if(bretunr){// BOTÂO DE RETORNO
        
        // Retorno para a tela principal se estiver na tela 1, 2 ou 3
        if(tela == 1 || tela == 2 || tela == 3) tela = 0;
        // Retorna para a tela de Configuração se estiver na tela 4
        if(tela == 4) {
            if(ctela_config <= 1){
                tela = 3;
            } else {
                
            }
            ctela_config = 1;
        }
        Escreve_texto_com_pos(cursor,1," ");
        cursor = 2;                          // Volta o cursor para a 2º posição
    }
    Escreve_texto_com_pos(cursor,1,">");     // Escreve a seta do cursor
}