/*
 * File:   ds18b20.h
 * Author: Jonnathan Alves
 * Adaptado de: https://www.youtube.com/watch?v=i6z4N2mcC4M
 * 
 * Created on 10 de Setembro de 2021, 18:03
 * 
 * Interface entre o PIC18F452 e o ds18b20
 * 
 * Pino para a comunicação OneWire: RA4
 * 
 */

// INCLUSÂO DA BIBLIOTECA DE COMUNICAÇÂO ONE WIRE_______________________________
# include "OneWire.h"

// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void ds18b20_init(char);
int  ds18b20_readTemp();

// SUBROTINA PARA A LEITURA DA TEMPERATURA______________________________________
int ds18b20_readTemp(){
    int temperatura;
    
    ds18b20_pulseReset();       // Pulso de Reset
    ds18b20_presensePulse();    // Pulso de Presença
    ds18b20_writeCommand(0xCC); // Comando skip rom
    ds18b20_writeCommand(0x44); // Inicia uma medição de temperatura
    __delay_ms(800);
    ds18b20_pulseReset();       // Pulso de Reset
    ds18b20_presensePulse();    // Pulso de Presença
    ds18b20_writeCommand(0xCC); // Comando skip rom
    ds18b20_writeCommand(0xBE); // Comando de Leitura da memória
    
    temperatura = ds18b20_readByte();                    // Lê o primeiro nibble
    temperatura = temperatura | ((ds18b20_readByte()) << 8); // Lê o segundo nib
    
    ds18b20_pulseReset();       // Pulso de Reset
    ds18b20_presensePulse();    // Pulso de Presença
    
    return temperatura;
}

// SUBROTINA PARA A INICIALIZAÇÃO E CONFIGURAÇÃODA PRECISÃO DO ds18b20__________
void ds18b20_init(char precision){
    char value = 0;
    switch(precision){
        case 9:
            value = 0b00011111; // 9 bits de precisão
            break;
        case 10:
            value = 0b00111111; // 10 bits de precisão
            break;
        case 11:
            value = 0b01011111; // 11 bits de precisão
            break;
        case 12:
            value = 0b01111111; // 12 bits de precisão
            break;
        default:    
            value = 0b00011111; // 9 bits de precisão como default
            break;
    }
    
    ds18b20_pulseReset();        // Pulso de Reset
    ds18b20_presensePulse();     // Pulso de Presença
    ds18b20_writeCommand(0xCC);  // Comando skip rom
    ds18b20_writeCommand(0x4E);  // Comando de configuração
    ds18b20_writeCommand(0x00);  // Escrevendo 0 no terceiro byte de memória
    ds18b20_writeCommand(0x00);  // Escrevendo 0 no quarto byte de memória
    ds18b20_writeCommand(value); // Escrevendo a precisão no quinto byte de mem
    ds18b20_pulseReset();        // Pulso de Reset     
    ds18b20_presensePulse();     // Pulso de Presença
}