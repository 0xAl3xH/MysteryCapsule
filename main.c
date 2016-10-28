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
void setupADC(void);
void setupUART(uint32_t baud);
void sendByte(uint8_t byte);
void sendBytePrintf(uint8_t byte, FILE *stream);
uint8_t recieveByte(void);
uint16_t adcRead(void);

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
}

/**
 * Sets up ADC 
 */
void setupADC(void) {
    ADCSRA |= (3 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); //set prescaler to 64
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

int main(void)
{
    setup();
    uint8_t recieved_byte;
    for(;;){
        recieved_byte = recieveByte();
        if (recieved_byte == 13) {
            for (int i = 0; i < 1000; i++) {
                uint16_t result = adcRead();
                printf("%u,", result);
                printf("%u\n", result & 0x0001);
            }
        }
    }
    return 0;   /* never reached */
}
