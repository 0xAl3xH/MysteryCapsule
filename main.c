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
#include "util/UART.h"

void setup(void);
void setupADC(void);
void sendBytePrintf(uint8_t byte, FILE *stream);
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
}

/**
 * Sets up ADC 
 */
void setupADC(void) {
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); //set prescaler to 64
    ADCSRA |= (1 << ADEN); //Enable ADC 
}

/**
 * Used to print streams to UART
 */
void sendBytePrintf(uint8_t byte, FILE *stream) {
    if (byte == '\n')
        sendByte('\r');
    sendByte(byte);
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

const char logo2048[] PROGMEM = "   ___       __   __ __       __\n /\'___`\\   /\'__`\\/\\ \\\\ \\    /\'_ `\\\n/\\_\\ /\\ \\ /\\ \\/\\ \\ \\ \\\\ \\  /\\ \\L\\ \\\n\\/_/// /__\\ \\ \\ \\ \\ \\ \\\\ \\_\\/_> _ <_\n   // /_\\ \\\\ \\ \\_\\ \\ \\__ ,__\\/\\ \\L\\ \\\n  /\\______/ \\ \\____/\\/_/\\_\\_/\\ \\____/\n  \\/_____/   \\/___/    \\/_/   \\/___/\n\n";

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
     printf_P(PSTR("%S"),logo2048);
     //Move cursor down 9 times to offset for printBoard
     printf_P(PSTR("\n\n\n\n\n\n\n\n\n"));
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
