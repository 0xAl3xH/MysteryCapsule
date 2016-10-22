/* Name: main.c
 * Author: Alex Huang
 * 10/21/16
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/**
 * Setup function which is run once on startup 
 */
void setup(void) {
    DDRB = 0xDF; //Set up PB0-PB6 as output for LEDs 
}

int main(void)
{
    setup();
    for(;;){
        PORTB ^= 0x01;  //Toggle PB0 
        _delay_ms(10);
    }
    return 0;   /* never reached */
}
