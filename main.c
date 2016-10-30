/* Name: main.c
 * Author: Alex Huang
 * 10/21/16
 */
#define F_CPU 8000000
#define BAUD_RATE 9600

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include "util/Board.h"

void setup(void);
void setupADC(void);
void setupUART(uint32_t baud);
void sendByte(uint8_t byte);
void sendBytePrintf(uint8_t byte, FILE *stream);
uint8_t recieveByte(void);
uint16_t adcRead(void);
uint16_t getSeed(void);
Boolean spawnTwo(void);
void printBoard(Board *board);
void play2048(void);

//Set up stream to use to redirect stdout characters  to UART send function 
static FILE uartSTDOUT = FDEV_SETUP_STREAM(sendBytePrintf,NULL,_FDEV_SETUP_WRITE);

/**
 * Setup function which is run once on startup 
 */
void setup(void) {
    DDRB = 0xDF; //Set up PB0-PB6 as output for LEDs 
    setupUART(BAUD_RATE);
    //Bind stdout to print via UART
    stdout = &uartSTDOUT;
    setupADC();
    srandom(getSeed());
    _delay_ms(3000);
}

/**
 * Sets up ADC 
 */
void setupADC(void) {
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); //set prescaler to 64
    ADCSRA |= (1 << ADEN); //Enable ADC 
}

/**
 * Sets up hardware UART given baud rate
 */
void setupUART(uint32_t baud) {
    //Set the ubrr value to generate baud rate 
    uint16_t ubrr = (F_CPU)/((uint32_t)16 * baud) - 1; 
    //uint16_t ubrr = 51;
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr;
        
    //UBRR0H = 0;
    //UBRR0L = 51;    

    //Enable receiver and transmitter
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    //Set 8 data bits, 1 stop bit, no parity
    UCSR0C = (3<<UCSZ00);
}

/**
 * Used to print streams to UART
 */
void sendBytePrintf(uint8_t byte, FILE *stream) {
    if (byte == '\n')
        sendByte('\r');
    sendByte(byte);
}

void sendByte(uint8_t byte) {
    // wait until port is ready to be written to
    while( ( UCSR0A & ( 1 << UDRE0 ) ) == 0 ){}

    // write the byte to the serial port
    UDR0 = byte;
}

uint8_t recieveByte(void) {
    // wait until a byte is in the buffer  
    while( ( UCSR0A & ( 1 << RXC0 ) ) == 0 ){}

    // grab the byte from the serial port
    return UDR0;
}

uint16_t adcRead(void) {
    ADCSRA |= (1 << ADSC);  //Start a new conversion;
    while (ADCSRA & (1<<ADSC)){} //Wait until ADC conversion complete
    uint16_t retVal = ADCL; //Record the results
    retVal |= ADCH << 8;
    return retVal;
}

/**
 * Get a random integer 
 */
uint16_t getSeed(void) {
    uint16_t retVal = 0;
    for (int i = 0; i < 16; i++) {
        retVal |= (adcRead() & 1) << i;
    }
    return retVal;
}

/**
 * Helper function to determine whether to spawn 2 or 4.
 * Returns True if a 2 should be spawned.
 */
Boolean spawnTwo(void) {
    //Spawn two 90% of the time
    uint8_t prob = random() % 10;
    return !(prob == 9);
} 

const char padding[] PROGMEM = "        ";
/**
 * Helper method to print the board to UART
 */
void printBoard(Board *board) {
    //The following code will make heavy use of the printf_P function to save RAM space
    uint16_t boardVal;
    //Get cursor back to original position
    printf_P(PSTR("%c[9A"),27);

    printf_P(PSTR("%S#-------------------#\n"),padding);
    for (uint8_t row = 0; row < 4; row++) {
        printf_P(PSTR("%S|"),padding);
        for (uint8_t col = 0; col < 4; col++) {
            boardVal = board -> grid[row][col].value;
            if (boardVal)
                printf_P(PSTR("%4u|"),board -> grid[row][col].value); 
            else
                printf_P(PSTR("    |"));
        }
        printf_P(PSTR("\n"));
        printf_P(PSTR("%S#-------------------#\n"),padding);
    }
}

/**
 * 2048 Game 
 */
void play2048(void) { 
     Board board = Board_newBlankBoard();  
     //Put down two random tiles first
     Board_putRandom(&board, random(),spawnTwo());
     Board_putRandom(&board, random(),spawnTwo());
     uint8_t recievedByte;
     Direction dir;
     printBoard(&board);

     while (!Board_gameWon(&board)) {
         recievedByte = recieveByte();
         if (recievedByte == 'j') 
             dir = LEFT;
         else if (recievedByte == 'l')
             dir = RIGHT;
         else if (recievedByte == 'i')
             dir = UP;
         else if (recievedByte == 'k')
             dir = DOWN;
         else
             continue;
         if (Board_shift(dir,&board)) {
            Board_putRandom(&board, random(), spawnTwo());
            printBoard(&board);
         }
     }
}

int main(void)
{
    setup();
    for(;;){
        play2048();
    }
    return 0;   /* never reached */
}
