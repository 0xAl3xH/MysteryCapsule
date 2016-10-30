#ifndef ADC_H
#define ADC_H
#include <avr/io.h>

/*
 * Implementation of APIs that allow users to interact
 * with the ADC module on an ATMega328P.
 */

/*
 * Sets up the ADC running with prescaler hard coded
 * to 64. 
 */
void ADC_setup(void);

/*
 * Reads the ADC data register and return the result
 */
uint16_t ADC_read(void);
#endif
