#include <ADC.h>

/**
 * Sets up ADC 
 */
void ADC_setup(void) {
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); //set prescaler to 64
    ADCSRA |= (1 << ADEN); //Enable ADC 
}

uint16_t ADC_read(void) {
    ADCSRA |= (1 << ADSC);  //Start a new conversion;
    while (ADCSRA & (1<<ADSC)){} //Wait until ADC conversion complete
    uint16_t retVal = ADCL; //Record the results
    retVal |= ADCH << 8;
    return retVal;
}
