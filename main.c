/* Name: main.c
 * Author: Alex Huang
 * 10/21/16
 */
#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void setup(void);
void setupUART(uint16_t baud);

/**
 * Setup function which is run once on startup 
 */
void setup(void) {
    DDRB = 0xDF; //Set up PB0-PB6 as output for LEDs 
    setupUART(9600);
}

/**
 * Sets up hardware UART given baud rate up to 65536 
 */
void setupUART(uint16_t baud) {
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

int main(void)
{
    setup();
    char recieved_byte;
    for(;;){
        // wait until a byte is ready to read
        while( ( UCSR0A & ( 1 << RXC0 ) ) == 0 ){}
 
        // grab the byte from the serial port
        recieved_byte = UDR0;
       
        // wait until the port is ready to be written to
        while( ( UCSR0A & ( 1 << UDRE0 ) ) == 0 ){}
 
        // write the byte to the serial port
        UDR0 = recieved_byte;
    }
    return 0;   /* never reached */
}
