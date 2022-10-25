/*
 * File:   ds18b20.h
 * Author: Jonnathan Alves
 * Adaptado de: https://www.youtube.com/watch?v=i6z4N2mcC4M
 * 
 * Created on 10 de Setembro de 2021, 18:03
 * 
 * Interface entre o PIC18F452 e o ds18b20
 * 
 * Pino para a comunica��o OneWire: RA4
 * 
 */

// INCLUS�O DA BIBLIOTECA DE COMUNICA��O ONE WIRE_______________________________
# include "OneWire.h"

// PROT�TIPOS DAS SUBROTINAS____________________________________________________
void ds18b20_init(char);
int  ds18b20_readTemp();

// SUBROTINA PARA A LEITURA DA TEMPERATURA______________________________________
int ds18b20_readTemp(){
    int temperatura;
    
    ds18b20_pulseReset();       // Pulso de Reset
    ds18b20_presensePulse();    // Pulso de Presen�a
    ds18b20_writeCommand(0xCC); // Comando skip rom
    ds18b20_writeCommand(0x44); // Inicia uma medi��o de temperatura
    __delay_ms(800);
    ds18b20_pulseReset();       // Pulso de Reset
    ds18b20_presensePulse();    // Pulso de Presen�a
    ds18b20_writeCommand(0xCC); // Comando skip rom
    ds18b20_writeCommand(0xBE); // Comando de Leitura da mem�ria
    
    temperatura = ds18b20_readByte();                    // L� o primeiro nibble
    temperatura = temperatura | ((ds18b20_readByte()) << 8); // L� o segundo nib
    
    ds18b20_pulseReset();       // Pulso de Reset
    ds18b20_presensePulse();    // Pulso de Presen�a
    
    return temperatura;
}

// SUBROTINA PARA A INICIALIZA��O E CONFIGURA��ODA PRECIS�O DO ds18b20__________
void ds18b20_init(char precision){
    char value = 0;
    switch(precision){
        case 9:
            value = 0b00011111; // 9 bits de precis�o
            break;
        case 10:
            value = 0b00111111; // 10 bits de precis�o
            break;
        case 11:
            value = 0b01011111; // 11 bits de precis�o
            break;
        case 12:
            value = 0b01111111; // 12 bits de precis�o
            break;
        default:    
            value = 0b00011111; // 9 bits de precis�o como default
            break;
    }
    
    ds18b20_pulseReset();        // Pulso de Reset
    ds18b20_presensePulse();     // Pulso de Presen�a
    ds18b20_writeCommand(0xCC);  // Comando skip rom
    ds18b20_writeCommand(0x4E);  // Comando de configura��o
    ds18b20_writeCommand(0x00);  // Escrevendo 0 no terceiro byte de mem�ria
    ds18b20_writeCommand(0x00);  // Escrevendo 0 no quarto byte de mem�ria
    ds18b20_writeCommand(value); // Escrevendo a precis�o no quinto byte de mem
    ds18b20_pulseReset();        // Pulso de Reset     
    ds18b20_presensePulse();     // Pulso de Presen�a
}