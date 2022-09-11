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
void Write_time(unsigned char, unsigned char, unsigned char);
void Read_time(unsigned char*);

// _____________________________________________________________________________
void Write_time(unsigned char hour, unsigned char minutes, unsigned char seconds){
    I2C_start();
    I2C_write(0b11010000);  // Endereço do ds1307 com o bit 0 no final (indicando escrita)
    I2C_write(0x00);        // Endereço de memória para os segundos
    
    I2C_write(seconds);
    I2C_write(minutes);
    I2C_write(hour);
    
    I2C_stop();
}

void Read_time(unsigned char *time){
    
    I2C_start();
    I2C_write(0b11010000);   // Endereço do ds1307 COM O bit 0 no final (indicando escrita)
    I2C_write(0x00);         // Endereço de memória para os segundos
    I2C_repeated_start();
    I2C_write(0b11010001);   // Endereço do ds1307 COM O bit 1 no final (indicando leitura)
    
    nFloatToStr(time  , 0, I2C_read()); 
    nFloatToStr(time+2, 0, I2C_read()); 
    nFloatToStr(time+4, 0, I2C_read()); 
    
    //I2C_stop();
}

