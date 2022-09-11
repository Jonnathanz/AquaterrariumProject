/* 
 * File:   LCD_16x4.h
 * Author: Adaptado por RODRIGO RIMOLDI DE LIMA em 22/12/2020 12:20
 * 
 * Atualizada por Jonnathan Alves em 18/03/2021 17:46
 */

// DEFINIÇÕES___________________________________________________________________

// NOTA 1:  Biblioteca escrita para a comunicação em 4 bits
// NOTA 2:  Basta alterar a pinagem (Linhas 13 a 18) conforme a conveniência

#define RS              PORTDbits.RD3       // Define RS para controle do LCD
#define E               PORTDbits.RD2       // Define o E para o controle do LCD
#define D7              PORTDbits.RD7       // Definição para os pinos D7 a D4
#define D6              PORTDbits.RD6
#define D5              PORTDbits.RD5
#define D4              PORTDbits.RD4

// PROTÓTIPO DAS SUBROTINAS_____________________________________________________
void Inicializa_LCD();
void Envia_comando_LCD(char);
void Envia_dado_LCD(char);
void Limpa_display_LCD();
void Pos_inicial_LCD();
void Escreve_texto_LCD(char*);
void Escreve_texto_com_pos(int, int, char*);
void Posicao_LCD(int, int);
void Novo_caractere_LCD(char, char[]);
void Timer_LCD(void);

// SUBROTINA DE INICIALIZAÇÃO DO LED____________________________________________
void Inicializa_LCD(){
    TRISD = 0b00000010;
    RS = 0;                     // Limpa RS
    E = 0;                      // Limpa E
    // PORT_dados = 0;          // Limpa os pinos de dados
    D7 = 0;
    D6 = 0;
    D5 = 0;
    D4 = 0;
    __delay_ms(50);
    Envia_comando_LCD(0x02);    // Coloca o cursor do LCD na posição inicial
    Envia_comando_LCD(0x28);    // Configura a comunicação em 4 bits
    Envia_comando_LCD(0x0C);    // Configura o display ON e o cursor OFF
    Envia_comando_LCD(0x06);    // Desloca o cursor para a posição à direita
}

// SUBROTINA DE ENVIO DE COMANDOS_______________________________________________
void Envia_comando_LCD(char comando) {
    RS = 0;
    //PORT_dados = comando & 0xF0;
    D7 = (comando & ( 1 << 7 )) >> 7;
    D6 = (comando & ( 1 << 6 )) >> 6;
    D5 = (comando & ( 1 << 5 )) >> 5;
    D4 = (comando & ( 1 << 4 )) >> 4;
    Timer_LCD();
    D7 = (comando & ( 1 << 3 )) >> 3;
    D6 = (comando & ( 1 << 2 )) >> 2;
    D5 = (comando & ( 1 << 1 )) >> 1;
    D4 = comando & 1;
    Timer_LCD();
    __delay_ms(2);
}

// SUBROTINA DE ENVIO DE DADOS__________________________________________________
void Envia_dado_LCD(char dado){
    RS = 1;
    D7 = (dado & ( 1 << 7 )) >> 7;
    D6 = (dado & ( 1 << 6 )) >> 6;
    D5 = (dado & ( 1 << 5 )) >> 5;
    D4 = (dado & ( 1 << 4 )) >> 4;
    Timer_LCD();
    D7 = (dado & ( 1 << 3 )) >> 3;
    D6 = (dado & ( 1 << 2 )) >> 2;
    D5 = (dado & ( 1 << 1 )) >> 1;
    D4 = dado & 1;
    Timer_LCD();
    __delay_us(50);
}

// SUBROTINA DE LIMPEZA DO LCD__________________________________________________
void Limpa_display_LCD(){
    Envia_comando_LCD(0X01);
}

// SUBROTINA PARA ESCRITA DE TEXTOS_____________________________________________
void Escreve_texto_LCD(char *texto){
     while(*texto){ // Será executado enquanto a condição não for nula
        Envia_dado_LCD(*texto); // Envia o caractere ao LCD
        texto++;                // Muda para o próximo caractere
    }    
}

// SUBROTINA PARA ESCRITA DE TEXTOS COM POSIÇÃO PRÉ-DEFINIDA____________________
void Escreve_texto_com_pos(int linha, int coluna, char *texto ) {
    char pos;
    switch(linha){
        case 1:
            pos = 0x80 + (coluna - 1);
            break;
        case 2:
            pos = 0xC0 + (coluna - 1);
            break;  
        case 3:
            pos = 0x94 + (coluna - 1);
            break;
        case 4:
            pos = 0xD4 + (coluna - 1);  
            break;
        default:
            pos = 0x80 + (coluna - 1);
            break;
    }
    Envia_comando_LCD(pos);
    Escreve_texto_LCD(texto);
}

// SUBROTINA DE POSICIONAMENTO DO CURSOR EM UMA POSIÇÃO QUALQUER________________
void Posicao_LCD(int linha, int coluna) {
    char pos;
    switch(linha){
        case 1:
            pos = 0x80 + (coluna - 1);
            break;
        case 2:
            pos = 0xC0 + (coluna - 1);
            break;  
        case 3:
            pos = 0x94 + (coluna - 1);
            break;
        case 4:
            pos = 0xD4 + (coluna - 1);  
            break;
        default:
            pos = 0x80 + (coluna - 1);
            break;
    }
    Envia_comando_LCD(pos);
}

// SUBROTINA PARA CRIAÇÃO DE NOVOS CARACTERES___________________________________
void Novo_caractere_LCD(char endereco, char caractere[]) {
    int i;
    Envia_comando_LCD(0x40 + (endereco*8));
    for(i = 0; i < 8; i++) {
        Envia_dado_LCD(caractere[i]);
    }
}

// SUBROTINA DE TEMPORIZAÇÃO____________________________________________________
void Timer_LCD(void){
    E = 1;
    __delay_us(5);
    E = 0;
    __delay_us(5);
}