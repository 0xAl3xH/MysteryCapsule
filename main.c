/* Name: main.c
 * Author: Alex Huang
 * 10/21/16
 */
#define F_CPU 8000000
#define BAUD_RATE 9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>
#include "util/Board.h"
#include "util/UART.h"
#include "util/ADC.h"

void setupTimer1(void);
void setup(void);
uint16_t getSeed(void);
void welcomeMessage(void);
Boolean spawnTwo(void);
void printBoard(Board *board);
void play2048(void);
void ledPuzzle(void);

uint8_t *beat2048EEPA = 0;  //Address of EEPROM variable beat2048
uint8_t beat2048 = 0; 

const uint8_t secretMessage[][2] = {{4,2},{2,1},{6,2},{6,2},{2,1},
                              {4,3},{7,4},
                              {2,1},
                              {2,2},{8,2},{8,1},{8,1},{3,2},{7,3},{3,3},{5,3},{9,3}};
volatile uint8_t flashCount = 0;
uint8_t secretDigit;
/**
 * Interrupt handler for LED puzzle
 */
ISR (TIMER1_COMPA_vect)
{
    if (flashCount) {
        //Toggle LEDs
        if (!(flashCount % 2))
            PORTC = secretDigit << 2; //Use PC2 - PC5
        else
            PORTC = 0;
        flashCount --;
    }
}

/**
 * Setup function for 16 bit timer1 running at 8 Mhz
 * for 0.25 second triggers
 */
void setupTimer1(void) {
    OCR1A = 0x7A0;

    // Mode 4, CTC on OCR1A
    TCCR1B |= (1 << WGM12);

    //Set interrupt on compare match
    TIMSK1 |= (1 << OCIE1A);

    // set prescaler to 1024 and start the timer
    TCCR1B |= (1 << CS12) | (1 << CS10);

    // enable interrupts
    sei();

}

/**
 * Setup function which is run once on startup 
 */
void setup(void) {
    DDRC = 0x3C; // Set up PC2-PC5 as output for LEDs
    UART_setup(BAUD_RATE);

    // Set up stream to use to redirect stdout and stdin to UART 
    static FILE uartSTD = FDEV_SETUP_STREAM(UART_sendByteSTD,UART_recieveByteSTD,_FDEV_SETUP_RW);

    // Bind stdout and stdin 
    stdout = &uartSTD;
    stdin = &uartSTD;

    ADC_setup();
    srandom(getSeed());
    while (!eeprom_is_ready()) {}
    beat2048 = eeprom_read_byte(beat2048EEPA);
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

/**
 * Blocking method that gets user input, suppoting backspace deletion and 
 * visual user feedback. Takes a pointer to a string variable to be 
 * modified to the user input and the variable's length.
 */
void getInput (char *str, size_t size) {
    uint32_t index = 0;
    uint8_t recievedByte = UART_recieveByte();
    while (recievedByte != '\r' && recievedByte !='\n') {
        // check that the character isn't backspace or delete on mac 
        // and that there is enough space in the string buffer
        if ((recievedByte != 127) && (recievedByte != '\b') && (index <= size - 2)) {
            // check that character can be printed on screen
            if (recievedByte >= 32 && recievedByte <= 126) {
                UART_sendByte(recievedByte); 
                str[index] = recievedByte;
                index ++;
            }
        } else if ((recievedByte == 127 || recievedByte == '\b') && (index >= 1)) {
            // handle backspace or delete on mac by moving cursor backward and deleting character 
            printf_P(PSTR("%c[1D"),27);
            printf_P(PSTR("%c[K"),27);
            index--;
        } else {
            // ring the bell
            UART_sendByte(7);
        }
        recievedByte = UART_recieveByte();
    }
    printf_P(PSTR("\n"));
    // put a null character at the end of the string
    str[index] = '\0'; 
}

const char cakeArt[] PROGMEM = "                        #,\n                        ###\n                       ## ##\n                      ##  ##\n                       ####\n                         :\n                        #####\n                       ######\n                       ##  ##\n                       ##  ##\n                       ##  ##\n                       ##  ##########\n                       ##  #############\n                  #######  ###############\n              #############################\n        .###################################\n       #####################################;\n       ##                                 ##.\n       ##                                 ##\n       #####################################\n       ##                                 ##\n       ##      Tosci\'s icecream cake      ##\n       ##       For a special Hanna       ###\n    #####                                 #####\n   ### ##################################### ###\n  ###  ##                                 ##  ###\n  ##   ## ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,, ##   ##\n   ##  #####################################  ##\n    ##                                       ##\n     ####                                 ####\n       ######                         ######\n          ###############################\n";

/**
 * Initiates the welcome message sequence
 */
void welcomeMessage(void) {
    char name[7];
    printf_P(PSTR("%S\n"),cakeArt);
    printf_P(PSTR("Hello Hanna, happy belated birthday!"));
    getInput(name, sizeof name);
    //printf_P(PSTR("Your name is: %s\n"),name);
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

     while (!Board_gameWon(&board) && !Board_gameOver(&board)) {
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
     if (Board_gameOver(&board)) {
        printf("\nGame Lost\n"); 
     } 
     if (Board_gameWon(&board)) {
        printf("\nGame Won\n");
     }
}

/**
 * Routine for LED puzzle
 */
void ledPuzzle(void){
    uint8_t index = 0;
    uint8_t recievedByte;
    printf_P(PSTR("Password: *****************"));
    printf_P(PSTR("%c[17D"),27);
    flashCount = 2 * secretMessage[index][1];
    secretDigit = secretMessage[index][0]; 
    setupTimer1();
    while(1) {
        recievedByte = UART_recieveByte();
        // Move left 
        if ((recievedByte == 'j') && (index > 0)) {
            index --;
            // Move cursor back
            printf_P(PSTR("%c[D"),27);
        }
        // Move right
        else if ((recievedByte == 'l') && (index < 16)) {
            index ++;
            // Move cursor forward
            printf_P(PSTR("%c[C"),27);
        } else {
            continue;
        }
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            //Update the LED flash interrupt variables
            flashCount = 2 * secretMessage[index][1] + 1;  //A flash will be on & off, hence the 2x multiplier, +1 is to give a short pause before the lights start flashing
            secretDigit = secretMessage[index][0];
        }
    }
}

int main(void)
{
    setup();
    for(;;){
        printf("%u,%u", beat2048, eeprom_read_byte(beat2048EEPA + 1));
        if (UART_recieveByte() == 'a')
            eeprom_write_byte(beat2048EEPA, 69);
        printf("\n%u", eeprom_read_byte(beat2048EEPA));
        welcomeMessage();
        play2048();
        ledPuzzle();
    }
    return 0;   /* never reached */
}
