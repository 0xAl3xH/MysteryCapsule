/* Name: main.c
 * Author: Alex Huang
 * 10/21/16
 */
#define F_CPU 8000000
#define BAUD_RATE 9600

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

void setup(void);
void setupUART(uint32_t baud);
void sendByte(uint8_t byte);
void sendBytePrintf(uint8_t byte, FILE *stream);
uint8_t recieveByte();

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

uint8_t recieveByte() {
    // wait until a byte is in the buffer  
    while( ( UCSR0A & ( 1 << RXC0 ) ) == 0 ){}

    // grab the byte from the serial port
    return UDR0;
}

int main(void)
{
    setup();
    uint8_t recieved_byte;
    for(;;){
        recieved_byte = recieveByte();
        if (recieved_byte == 13) {
            sendByte('\n');
            sendByte('\r');
        } else {
            sendByte(recieved_byte);
        }
    }
    return 0;   /* never reached */
}
