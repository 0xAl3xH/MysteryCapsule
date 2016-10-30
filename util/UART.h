#ifndef UART_H
#define UART_H
#include <avr/io.h>
/*
 * Implementation of APIs that allow users to interact with 
 * the UART subsystem on an ATMega328P.
 */

/*
 * Given the baud rate, set up the UART. 
 */
void setupUART(uint32_t baud);

/*
 * Sends a character over the UART module
 * takes the byte to be sent as input.
 */
void sendByte(uint8_t byte);

/*
 * Recieves a character over the UART module
 * note that this method is blocking and will
 * not return until a character is recieved.
 * Returns the byte that has been recieved
 * in the UART module.
 */
uint8_t recieveByte(void); 
#endif
