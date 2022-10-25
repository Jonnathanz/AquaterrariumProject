/*
 * File:   OneWire.h
 * Author: Jonnathan Alves
 * Adaptado de: https://www.youtube.com/watch?v=i6z4N2mcC4M
 * 
 * Created on 31 de dezembro de 2021, 20:16
 * 
 * Comunicação OneWire com o PIC18F452
 * 
 * Pino para a comunicação OneWire: RA4
 * 
 */




// DEFINIÇÕES___________________________________________________________________
#define dataPin   PORTAbits.RA4
#define direction TRISAbits.TRISA4

// PROTÓTIPOS DAS SUBROTINAS____________________________________________________
void ds18b20_readTemperature(char);
void ds18b20_pulseReset(void);
void ds18b20_presencaPulse(void);
void ds18b20_writeZero(void);
void ds18b20_writeOne(void);
void ds18b20_writeCommand(char);
char ds18b20_readBit(void);
char ds18b20_readByte(void);

// VARIÁVEIS GLOBAIS____________________________________________________________
static unsigned int delay_ds18b20_us;  // Arrumar depois

// SUBROTINA PARA O PULSO DE RESET______________________________________________
void ds18b20_pulseReset(){
    direction   = 0;      // Pino como saída
    dataPin     = 0;      // Saída do pino com nível baixo
    __delay_us(500);      // Mínimo de 480 us
    dataPin     = 1;      // Pino como entrada
    __delay_us(60);       // Tempo para o sensor mandar o pulso de resposta
}

// SUBROTINA PARA A DETECÇÃO DO PULSO DE PRESENÇA_______________________________
void ds18b20_presensePulse(){
    direction     = 1;  // Pino como entrada
    // VERIFICAR DEPOIS O QUE ESTÁ COMENTADO
    // PARECE QUE O WHILE AS VEZES BUGA E NÃO SAI DELE
    // PODE SER POR CAUSA DE ALGUMA INTERRUPÇÃO
    //while (dataPin);    // Espera até o ds18b20 colocar o barramento em Low 
    //while (!dataPin);   // Espera até o ds18b20 colocar o barramento em High 
    //__delay_us(500);    // Tempo mínimo de espera de 480us
    __delay_us(1000);   // Tempo mínimo de espera de 480us
    direction     = 0;  // Pino como saída
    
}

// SUBROTINA PARA A ESCRITA DO BIT 0____________________________________________
void ds18b20_writeZero(){
    direction   = 0;    // Configuração do Pino como saída
    dataPin     = 0;    // Saída do pino como nível baixo
    __delay_us(90);     // Tempo mínimo de 60 us como nível baixo
    dataPin     = 1;    // Saída do pino como nível alto
    __delay_us(10);     // Tempo mínimo do TimeSlot de escrita de 60 us
}

// SUBROTINA PARA A ESCRITA DO BIT 1____________________________________________
void ds18b20_writeOne(){
    direction   = 0;    // Configuração do Pino como saída
    dataPin     = 0;    // Saída do pino como nível baixo
    __delay_us(5);      // Tempo máximo de até de 15 us como nível baixo
    dataPin     = 1;    // Saída do pino como nível alto
    __delay_us(90);     // Tempo mínimo do TimeSlot de escrita de 60 us
}

// SUBROTINA PARA A ESCRITA DE UM COMANDO_______________________________________
void ds18b20_writeCommand(char command){
    for(int i = 0; i < 8; i++){
        if(command & (0x01))     // Se o primeiro bit for High
            ds18b20_writeOne();  // Envia o Bit 1
        else                     // Se o primeiro bit for Low
            ds18b20_writeZero(); // Envia o Bit 0
        command = command >> 1;  // Move o byte para a esquerda
                                 // para analisar o bit seguinte
    }
    __delay_us(10);
}

// SUBROTINA PARA A LEITURA DE UM BIT___________________________________________
char ds18b20_readBit(){
    char _bit = 0;
    
    direction   = 0;    // Configuração do pino como saída
    dataPin     = 0;    // Saída do pino como nível baixo
    __delay_us(3);      // Tempo mínimo de 1us como nível baixo
    direction   = 1;    // Configuração do pino como entrada
    __delay_us(3);      // Tempo de espera para a estabilização
    _bit = dataPin;     // Leitura do pino
    __delay_us(90);     // Tempo mínimo do TimeSlot de leitura de 60 us
    
    return _bit;
}

// SUBROTINA PARA A LEITURA DE UM BYTE__________________________________________
char ds18b20_readByte(void){
    char _byte  = 0x00;
    
    for(int i=0; i < 8; i++)
        _byte = _byte | (ds18b20_readBit() << i);
    
    return _byte;
}