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
#include "text.h"

void EEPROM_Write(uint8_t *addr, uint8_t data);
uint8_t EEPROM_Read(uint8_t *addr);
void setupTimer1(void);
void setup(void);
uint16_t getSeed(void);
void messageSequence(const char ** messages, uint8_t size);
void welcomeMessage(void);
Boolean spawnTwo(void);
void printBoard(Board *board);
void play2048(void);
void ledPuzzle(void);
void selectGame(void);
void birthdayMessage(void); 

uint8_t *accessLevelAddr = (uint8_t *) 0;  //Address of EEPROM variable beat2048
uint8_t accessLevel = 0; 
uint8_t *messageAuthAddr = (uint8_t *) 1;  //Address of EEPROM variable messageAuth
uint8_t messageAuth = 0;

const uint8_t secretMessage[][2] = {{4,2},{2,1},{6,2},{6,2},{2,1},
                              {4,3},{7,4},
                              {2,1},
                              {2,2},{8,2},{8,1},{8,1},{3,2},{7,3},{3,3},{5,3},{9,3}};
volatile uint8_t flashCount = 0;
uint8_t secretDigit;

void EEPROM_Write(uint8_t *addr, uint8_t data) {
    while (!eeprom_is_ready()) {}
    eeprom_write_byte(addr,data);
}

uint8_t EEPROM_Read(uint8_t *addr) {
    while (!eeprom_is_ready()) {}
    return eeprom_read_byte(addr);
}

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

    //Set up UART
    UART_setup(BAUD_RATE);
    // Set up stream to use to redirect stdout and stdin to UART 
    static FILE uartSTD = FDEV_SETUP_STREAM(UART_sendByteSTD,UART_recieveByteSTD,_FDEV_SETUP_RW);
    // Bind stdout and stdin 
    stdout = &uartSTD;
    stdin = &uartSTD;
    printf_P(PSTR("%c[2J%c[H"),27,27);  // Home cursor and clear screen

    ADC_setup();
    srandom(getSeed());
    accessLevel = EEPROM_Read(accessLevelAddr);
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
 * Blocking method that waits for user to input 'enter' key
 */
void getEnter(void) {
    uint8_t recievedByte = UART_recieveByte();
    while (recievedByte != '\r' && recievedByte !='\n')
        recievedByte = UART_recieveByte();
}

/**
 * Blocking method that gets user input, suppoting backspace deletion and 
 * visual user feedback. Takes a pointer to a string variable to be 
 * modified to the user input and the variable's length.
 * The str argument should always have a length of at least 2.
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

void messageSequence(const char ** messages, uint8_t size) { 
    for (int i =0; i < size; i++) {
        printf_P(messages[i]);
        getEnter();
        printf_P(PSTR("\n\n"));
    }
}

/**
 * Initiates the welcome message sequence
 */
void welcomeMessage(void) {
    const char* messages[] = {welcome1, welcome2, welcome3,cakeArt, welcome4, welcome5, welcome6, welcome7, welcome8, welcome9};
    messageSequence(messages, 10);
    EEPROM_Write(accessLevelAddr, 1);    
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


/**
 * Helper method to print the board to UART
 */
void printBoard(Board *board) {
    //The following code will make heavy use of the printf_P function to save RAM space
    uint16_t boardVal;
    //VT100 escape sequence to get cursor back to original position
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
    printf_P(PSTR("%c[2J%c[H"),27,27);  //Clears screen, home cursor
    printf_P(logo2048);
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
           if(Board_gameOver(&board)) {
               //start a new game if game over
               printf_P(PSTR("Damn, game over. Try again?"));
               getEnter();
               printf_P(PSTR("\r%c[K"),27);
               board = Board_newBlankBoard();
               Board_putRandom(&board, random(),spawnTwo());
               Board_putRandom(&board, random(),spawnTwo());
               printBoard(&board);
           } 
        }
    }
    EEPROM_Write(accessLevelAddr, 2);   
}

/**
 * Routine for LED puzzle
 */
void ledPuzzle(void){
    uint8_t index = 0;
    uint8_t recievedByte;
    const char * ledMessages[] = {led1, led2, led3, led4};
    messageSequence(ledMessages,4);
    printf_P(PSTR("-----ENCRYPTED PASSWORD-----\n"));
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

void selectGame(void) {
    char response[2]; 
    if (accessLevel == 1) {
        //Skip welcome message?
        printf_P(PSTR("It seems like you've read through the welcome message before. Would you like to skip to 2048? [y/n]? "));
        getInput(response,sizeof response); 
        if (strcmp_P(response,PSTR("y")) == 0) {
            play2048();
            ledPuzzle();
        } else {
            return;
        }
    } else if (accessLevel == 2) {
        //Skip welcome message or 2048? or input unlock code
        printf_P(PSTR("It seems like you've beaten 2048, would you like to:\n  1.Play the entire game again?\n  2.Play 2048?\n  3.Look at the LED Puzzle?\n  4.Enter password/read birthday message copy?\n[1-4]:"));
        getInput(response, sizeof response);
        if (strcmp_P(response, PSTR("1")) == 0) {
            return;
        } else if (strcmp_P(response, PSTR("2")) == 0) {
            play2048();
            ledPuzzle();
        } else if (strcmp_P(response, PSTR("3")) == 0) {
            ledPuzzle(); 
        } else if (strcmp_P(response, PSTR("4")) == 0) {
            birthdayMessage();
        } else {
            return;
        }
    }
}

void birthdayMessage(void) {
    if (!EEPROM_Read(messageAuthAddr)) {
        char response[20];
        Boolean unauthorized = TRUE;
        while (unauthorized) {
            printf_P(PSTR("Input password to get copy of birthday message: "));
            getInput(response, sizeof response);
            if (strcmp_P(response, MESSAGE_PASSWORD) == 0) {
                EEPROM_Write(messageAuthAddr, 1);
                unauthorized = FALSE;
                printf_P(PSTR("\n"));
            } else {
                printf_P(PSTR("Invalid password, try again\n"));
            }
        }
    }    
    const char * messages[] = {birthday1,birthday2,birthday3,birthday4};    
    messageSequence(messages,4);
    while(1) {}
}

int main(void)
{
    setup();
    for(;;){ 
        selectGame();
        welcomeMessage();
        play2048();
        ledPuzzle();
    }
    return 0;   /* never reached */
}
