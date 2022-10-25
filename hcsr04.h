/*  
 * File: hcsr04.h
 * Author: Jonnathan Alves
 * 
 * Created on 18 de Mar�o de 2021, 16:52
 * 
 * BIBLIOTECA PARA A CONFIGURA��O E OPERA��O DE DOIS SENSORES ULTRASS�NICOS
 * UTILIZANDO O TIMER1 E TIMER3 E M�DULOS CCP1 E CCP2
 * 
 *      1� hcsr04: Medi��o de n�vel de ra��o: Utiliza o timer1
 *      2� hcsr04: Medi��o de n�vel da �gua:  Utiliza o timer3
 * 
 */

// DEFINICOES___________________________________________________________________
#define trigger1    PORTCbits.RC0
#define trigger2    PORTDbits.RD0

// VARI�VEIS GLOBAIS____________________________________________________________
unsigned char tempo_h1, tempo_l1;   // tempos no timer1
unsigned int periodo_1;             // per�odo de echo do sensor 1
float distancia_1;                  // dist�ncia medida no hcsr04_1

unsigned char tempo_h2, tempo_l2;   // tempo no timer3
unsigned int periodo_2;             // per�odo de echo do sensor 1
float distancia_2;                  // dist�ncia medida no hcsr04_12

// PROT�TIPOS DAS SUBROTINAS____________________________________________________
void inicializa_hcsr04(void); 
void trigger_1(void);
void distance_1(void);

void trigger_2(void);
void distance_2(void);

// SUBROTINA PARA A INICIALIZA��O DOS HCSR04____________________________________
void inicializa_hcsr04(void){
    TRISCbits.RC0 = 0;
    TRISDbits.RD0 = 0;
    TRISCbits.RC1 = 1;
    TRISCbits.RC2 = 1;
    TRISBbits.RB3 = 1;
}

// SUBROTINA PARA O DISPARO DO PRIMEIRO HCSR04__________________________________
void trigger_1(void){
    trigger1 = 1;
    __delay_us(20);
    trigger1 = 0;
}

// SUBROTINA PARA A MEDI��O DE DIST�NCIA DO PRIMEIRO HCSR04_____________________
void distance_1(void){
    //distancia_1 = 0;
    __delay_ms(100);
    trigger_1();        // Dispara o gatilho do sensor
    CCP1IE      = 1;
    CCP1IF      = 0;
    __delay_ms(100);
    periodo_1   = (tempo_h1 << 8) + tempo_l1;   // periodo a cada 0.8us
    distancia_1 = 0.8*periodo_1/58;             // converte a dist�ncia para cm
}

// SUBROTINA PARA O DISPARO DO SEGUNDO HCSR04___________________________________
void trigger_2(void){
    trigger2 = 1;
    __delay_us(20);
    trigger2 = 0;
}

// SUBROTINA PARA A MEDI��O DE DIST�NCIA DO SEGUNDO HCSR04______________________
void distance_2(void){
    trigger_2();        // Dispara o gatilho do sensor
    CCP2IE      = 1;
    CCP2IF      = 0;
    __delay_ms(100);
    periodo_2   = (tempo_h2 << 8) + tempo_l2;   // periodo a cada 0.8us
    distancia_2 = 0.8*periodo_2/58;             // converte a dist�ncia para cm
}
