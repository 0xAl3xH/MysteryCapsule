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
#include "util/ADC.h"

void setup(void);
uint16_t getSeed(void);
void welcomeMessage(void);
Boolean spawnTwo(void);
void printBoard(Board *board);
void play2048(void);

/**
 * Setup function which is run once on startup 
 */
void setup(void) {
    DDRB = 0xDF; // Set up PB0-PB6 as output for LEDs 
    UART_setup(BAUD_RATE);

    // Set up stream to use to redirect stdout and stdin to UART 
    static FILE uartSTD = FDEV_SETUP_STREAM(UART_sendByteSTD,UART_recieveByteSTD,_FDEV_SETUP_RW);

    // Bind stdout and stdin 
    stdout = &uartSTD;
    stdin = &uartSTD;

    ADC_setup();
    srandom(getSeed());
}

/**
 * Get a random integer 
 */
uint16_t getSeed(void) {
    uint16_t retVal = 0;
    for (int i = 0; i < 16; i++) {
        retVal |= (ADC_read() & 1) << i;
    }
    return retVal;
}

void getInput (char *str, size_t size) {
    uint32_t index = 0;
    uint8_t recievedByte = UART_recieveByte();
    while (recievedByte != 13) {
        if ((recievedByte != 127) && (recievedByte != 8) && (index <= size - 2)) {
            if (recievedByte >= 32 && recievedByte <= 126) {
                UART_sendByte(recievedByte); 
                str[index] = recievedByte;
                index ++;
            }
        } else if ((recievedByte == 127 || recievedByte == 8) && (index >= 1)) {
            printf_P(PSTR("%c[1D"),27);
            printf_P(PSTR("%c[K"),27);
            index--;
        } else {
            UART_sendByte(7);
        }
        recievedByte = UART_recieveByte();
    }
    printf_P(PSTR("\n"));
    str[index] = '\0'; 
}

/**
 * Initiates the welcome message sequence
 */
void welcomeMessage(void) {
    char name[7];
    printf_P(PSTR("Enter your name:"));
    //fgets(name,sizeof name, stdin);
    getInput(name, sizeof name);
    printf_P(PSTR("Your name is: %s\n"),name);
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
         recievedByte = UART_recieveByte();
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
        welcomeMessage();
        play2048();
    }
    return 0;   /* never reached */
}
