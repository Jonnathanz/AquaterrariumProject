/*
 * File:   I2C.c
 * Author: Jonnathan Alves
 *
 * Created on 04 de Setembro de 2021, 22:03
 * 
 * Configuração e funções da iluminacao
 * 
 * Iluminacao é programada para ficar 20 minutos ligado a partir do horário
 * pré-definido
 * 
 * https://electrosome.com/i2c-pic-microcontroller-mplab-xc8/
 * 
 * https://openlabpro.com/guide/i2c-module-in-pic18f4550/
 */



// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void I2C_init(unsigned int);
void I2C_idle(void);
void I2C_wait(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_repeated_start(void);
unsigned char I2C_read(void);
void I2C_write(unsigned char);

// _____________________________________________________________________________
void I2C_init(unsigned int clock){
    SSPCON1       = 0b00101000;
    SSPCON2       = 0b00000000;
    SSPSTAT       = 0b00000000;
    SSPADD        = _XTAL_FREQ/(4*clock) - 1;
    TRISCbits.RC3 = 1;
    TRISCbits.RC4 = 1;
    SSPIF = 0;
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
unsigned char I2C_read(void){
    unsigned char data;
    
    I2C_idle();
    SSPCON2bits.RCEN = 1;
    while(!BF);         //verificar este while depois
    SSPCON2bits.RCEN = 0;
    //I2C_idle();
    data             = SSPBUF;
    //I2C_idle();
    // ACKDT = 1;
    ACKEN = 1;
    I2C_wait();
    
    return data;
}