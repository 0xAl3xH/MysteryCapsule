#include <UART.h>

void UART_setup(uint32_t baud) {
    //Set the ubrr value to generate baud rate 
    uint16_t ubrr = (F_CPU)/((uint32_t)16 * baud) - 1;         
    UBRR0H = ubrr >> 8;                                        
    UBRR0L = ubrr;                                             
        
    //Enable receiver and transmitter                          
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    //Set 8 data bits, 1 stop bit, no parity                   
    UCSR0C = (3<<UCSZ00);                                      
}  

void UART_sendByte(uint8_t byte) {
    // wait until port is ready to be written to
    while( ( UCSR0A & ( 1 << UDRE0 ) ) == 0 ){}

    // write the byte to the serial port
    UDR0 = byte;
}

uint8_t UART_recieveByte(void) {
    // wait until a byte is in the buffer  
    while( ( UCSR0A & ( 1 << RXC0 ) ) == 0 ){}

    // grab the byte from the serial port
    return UDR0;
}

void UART_sendByteSTD(uint8_t byte, FILE *stream) {
        if (byte == '\n')
                    UART_sendByte('\r');
            UART_sendByte(byte);
}

uint8_t UART_recieveByteSTD(FILE *stream) {
    uint8_t recievedChar = UART_recieveByte();
    UART_sendByte(recievedChar);
    return recievedChar;
}
