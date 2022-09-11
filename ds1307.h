/*
 * File:   I2C.c
 * Author: Jonnathan Alves
 *
 * Created on 04 de Setembro de 2021, 22:03
 * 
 * Configura��o e fun��es da iluminacao
 * 
 * Iluminacao � programada para ficar 20 minutos ligado a partir do hor�rio
 * pr�-definido
 * 
 * https://electrosome.com/i2c-pic-microcontroller-mplab-xc8/
 * 
 * https://openlabpro.com/guide/i2c-module-in-pic18f4550/
 */

// PROT�TIPOS DAS SUBROTINAS____________________________________________________
void Write_time(unsigned char, unsigned char, unsigned char);
void Read_time(unsigned char*);

// _____________________________________________________________________________
void Write_time(unsigned char hour, unsigned char minutes, unsigned char seconds){
    I2C_start();
    I2C_write(0b11010000);  // Endere�o do ds1307 com o bit 0 no final (indicando escrita)
    I2C_write(0x00);        // Endere�o de mem�ria para os segundos
    
    I2C_write(seconds);
    I2C_write(minutes);
    I2C_write(hour);
    
    I2C_stop();
}

void Read_time(unsigned char *time){
    
    I2C_start();
    I2C_write(0b11010000);   // Endere�o do ds1307 COM O bit 0 no final (indicando escrita)
    I2C_write(0x00);         // Endere�o de mem�ria para os segundos
    I2C_repeated_start();
    I2C_write(0b11010001);   // Endere�o do ds1307 COM O bit 1 no final (indicando leitura)
    
    nFloatToStr(time  , 0, I2C_read()); 
    nFloatToStr(time+2, 0, I2C_read()); 
    nFloatToStr(time+4, 0, I2C_read()); 
    
    //I2C_stop();
}

