/*
 * File:   OneWire.h
 * Author: Jonnathan Alves
 * Adaptado de: https://www.youtube.com/watch?v=i6z4N2mcC4M
 * 
 * Created on 31 de dezembro de 2021, 20:16
 * 
 * Comunica��o OneWire com o PIC18F452
 * 
 * Pino para a comunica��o OneWire: RA4
 * 
 */




// DEFINI��ES___________________________________________________________________
#define dataPin   PORTAbits.RA4
#define direction TRISAbits.TRISA4

// PROT�TIPOS DAS SUBROTINAS____________________________________________________
void ds18b20_readTemperature(char);
void ds18b20_pulseReset(void);
void ds18b20_presencaPulse(void);
void ds18b20_writeZero(void);
void ds18b20_writeOne(void);
void ds18b20_writeCommand(char);
char ds18b20_readBit(void);
char ds18b20_readByte(void);

// VARI�VEIS GLOBAIS____________________________________________________________
static unsigned int delay_ds18b20_us;  // Arrumar depois

// SUBROTINA PARA O PULSO DE RESET______________________________________________
void ds18b20_pulseReset(){
    direction   = 0;      // Pino como sa�da
    dataPin     = 0;      // Sa�da do pino com n�vel baixo
    __delay_us(500);      // M�nimo de 480 us
    dataPin     = 1;      // Pino como entrada
    __delay_us(60);       // Tempo para o sensor mandar o pulso de resposta
}

// SUBROTINA PARA A DETEC��O DO PULSO DE PRESEN�A_______________________________
void ds18b20_presensePulse(){
    direction     = 1;  // Pino como entrada
    // VERIFICAR DEPOIS O QUE EST� COMENTADO
    // PARECE QUE O WHILE AS VEZES BUGA E N�O SAI DELE
    // PODE SER POR CAUSA DE ALGUMA INTERRUP��O
    //while (dataPin);    // Espera at� o ds18b20 colocar o barramento em Low 
    //while (!dataPin);   // Espera at� o ds18b20 colocar o barramento em High 
    //__delay_us(500);    // Tempo m�nimo de espera de 480us
    __delay_us(1000);   // Tempo m�nimo de espera de 480us
    direction     = 0;  // Pino como sa�da
    
}

// SUBROTINA PARA A ESCRITA DO BIT 0____________________________________________
void ds18b20_writeZero(){
    direction   = 0;    // Configura��o do Pino como sa�da
    dataPin     = 0;    // Sa�da do pino como n�vel baixo
    __delay_us(90);     // Tempo m�nimo de 60 us como n�vel baixo
    dataPin     = 1;    // Sa�da do pino como n�vel alto
    __delay_us(10);     // Tempo m�nimo do TimeSlot de escrita de 60 us
}

// SUBROTINA PARA A ESCRITA DO BIT 1____________________________________________
void ds18b20_writeOne(){
    direction   = 0;    // Configura��o do Pino como sa�da
    dataPin     = 0;    // Sa�da do pino como n�vel baixo
    __delay_us(5);      // Tempo m�ximo de at� de 15 us como n�vel baixo
    dataPin     = 1;    // Sa�da do pino como n�vel alto
    __delay_us(90);     // Tempo m�nimo do TimeSlot de escrita de 60 us
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
    
    direction   = 0;    // Configura��o do pino como sa�da
    dataPin     = 0;    // Sa�da do pino como n�vel baixo
    __delay_us(3);      // Tempo m�nimo de 1us como n�vel baixo
    direction   = 1;    // Configura��o do pino como entrada
    __delay_us(3);      // Tempo de espera para a estabiliza��o
    _bit = dataPin;     // Leitura do pino
    __delay_us(90);     // Tempo m�nimo do TimeSlot de leitura de 60 us
    
    return _bit;
}

// SUBROTINA PARA A LEITURA DE UM BYTE__________________________________________
char ds18b20_readByte(void){
    char _byte  = 0x00;
    
    for(int i=0; i < 8; i++)
        _byte = _byte | (ds18b20_readBit() << i);
    
    return _byte;
}