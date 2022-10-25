/*  
 * File: hcsr04.h
 * Author: Jonnathan Alves
 * 
 * Created on 18 de Março de 2021, 16:52
 * 
 * BIBLIOTECA PARA A CONFIGURAÇÃO E OPERAÇÃO DE DOIS SENSORES ULTRASSÔNICOS
 * UTILIZANDO O TIMER1 E TIMER3 E MÓDULOS CCP1 E CCP2
 * 
 *      1º hcsr04: Medição de nível de ração: Utiliza o timer1
 *      2º hcsr04: Medição de nível da água:  Utiliza o timer3
 * 
 */

// DEFINICOES___________________________________________________________________
#define trigger1    PORTCbits.RC0
#define trigger2    PORTDbits.RD0

// VARIÁVEIS GLOBAIS____________________________________________________________
unsigned char tempo_h1, tempo_l1;   // tempos no timer1
unsigned int periodo_1;             // período de echo do sensor 1
float distancia_1;                  // distância medida no hcsr04_1

unsigned char tempo_h2, tempo_l2;   // tempo no timer3
unsigned int periodo_2;             // período de echo do sensor 1
float distancia_2;                  // distância medida no hcsr04_12

// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void inicializa_hcsr04(void); 
void trigger_1(void);
void distance_1(void);

void trigger_2(void);
void distance_2(void);

// SUBROTINA PARA A INICIALIZAÇÃO DOS HCSR04____________________________________
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

// SUBROTINA PARA A MEDIÇÃO DE DISTÂNCIA DO PRIMEIRO HCSR04_____________________
void distance_1(void){
    //distancia_1 = 0;
    __delay_ms(100);
    trigger_1();        // Dispara o gatilho do sensor
    CCP1IE      = 1;
    CCP1IF      = 0;
    __delay_ms(100);
    periodo_1   = (tempo_h1 << 8) + tempo_l1;   // periodo a cada 0.8us
    distancia_1 = 0.8*periodo_1/58;             // converte a distância para cm
}

// SUBROTINA PARA O DISPARO DO SEGUNDO HCSR04___________________________________
void trigger_2(void){
    trigger2 = 1;
    __delay_us(20);
    trigger2 = 0;
}

// SUBROTINA PARA A MEDIÇÃO DE DISTÂNCIA DO SEGUNDO HCSR04______________________
void distance_2(void){
    trigger_2();        // Dispara o gatilho do sensor
    CCP2IE      = 1;
    CCP2IF      = 0;
    __delay_ms(100);
    periodo_2   = (tempo_h2 << 8) + tempo_l2;   // periodo a cada 0.8us
    distancia_2 = 0.8*periodo_2/58;             // converte a distância para cm
}
