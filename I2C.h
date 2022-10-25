/*
 * File:   I2C.c
 * Author: Jonnathan Alves
 *
 * Created on 04 de Setembro de 2021, 22:03
 * 
 * Configuração do protocolo de comunicação I2C com o microcontrolador como 
 * mestre
 * 
 * Pino RC3: SCL
 * Pino RC4: SDA
 * 
 */

// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void I2C_init(unsigned long);
void I2C_idle(void);
void I2C_wait(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_repeated_start(void);
unsigned char I2C_read(unsigned char);
void I2C_write(unsigned char);

// _____________________________________________________________________________
void I2C_init(unsigned long clock){
    SSPCON1       = 0b00101000;
    SSPCON2       = 0b00000000;
    SSPSTAT       = 0b00000000;
    SSPADD        = _XTAL_FREQ/(4*clock) - 1; // Clock na frequência
    TRISCbits.RC3 = 1;                        // RC3 como entrada
    TRISCbits.RC4 = 1;                        // RC4 como entrada
    SSPIF = 0;                                // Flag da interrupção I2C em Low
}

// _____________________________________________________________________________
void I2C_start(void){
    I2C_idle();
    SSPCON2bits.SEN = 1;
    I2C_wait();
}

// _____________________________________________________________________________
void I2C_repeated_start(void){
    I2C_idle();
    SSPCON2bits.RSEN = 1;
}

// _____________________________________________________________________________
void I2C_stop(void){
    I2C_idle();
    SSPCON2bits.PEN = 1;
    I2C_wait();
}

// _____________________________________________________________________________
void I2C_idle(void){
    while((SSPCON2 & 0b00011111) || (SSPSTAT & 0b00000100));
}

// _____________________________________________________________________________
void I2C_wait(void){
    while(!SSPIF);
    SSPIF = 0;
}

// _____________________________________________________________________________
void I2C_write(unsigned char data){
    I2C_idle();
    SSPBUF = data;
    I2C_wait();
}

// _____________________________________________________________________________
unsigned char I2C_read(unsigned char c){
    unsigned char data;
    
    I2C_idle();
    SSPCON2bits.RCEN = 1;
    while(!BF);         //verificar este while depois
    SSPCON2bits.RCEN = 0;
    data             = SSPBUF;

    ACKDT = c;      //Verificar depois se é negado ou nao
    ACKEN = 1;      
    I2C_wait();
    
    return data;
}