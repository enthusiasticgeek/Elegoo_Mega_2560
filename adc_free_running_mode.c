//(c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// http://www.electroschematics.com/10053/avr-adc/
//Benn Thomsen has a good article on this. This is the basis for the following program. See below.
// Reference: https://bennthomsen.wordpress.com/arduino/peripherals/analogue-input/

//Atmega 2560 has 10-bit successive approximation type ADC @ 15kS/s sampling rate.
//ADC modes: Single Conversion, Triggered Conversion, Free Running (We use the last one here)

/*
From Atmega2560 datasheet

    ADC Prescaler: 7-bit Selected using ADPS0, ADPS1, ADPS2 (2, 4, 16, 32, 64 and 128) - ADC frequency between 50KHz to 200KHz (> 10 bit resolution)
    ADC Channels: The ADC in Atmega2560 PDIP package has 16 channels, allows you to take samples from 16 different pins
    ADC Registers: Register provides the communication link between CPU and the ADC. You can configure the ADC according to your need using these registers. The ADC has 3 registers only:
        ADC Multiplexer Selection Register – ADMUX: For selecting the reference voltage and the input channel
        ADC Control and Status Register A – ADCSRA: It has the status of ADC and is also used to control it
        The ADC Data Register – ADCL and ADCH: Final result of the conversion is stored here
    AVCC: This pin supplies power to ADC. AVCC must not differ more than ± 0.3V from Vcc
    AREF: Another pin which can optionally be used as an external voltage reference pin.
    Voltage Resolution:This is the smallest voltage increment which can be measured. For 10 bit ADC, there can be 1024 different voltages (for an 8 bit ADC, there can be 256 different voltages)

*/

//Connect VCC to 330 Ohms to +ve end of LED and -ve end of LED to pin 7 of Arduino
//Connect VCC to 330 Ohms to +ve end of LED and -ve end of LED to pin 6 of Arduino
//Connect 10K Ohms POTENTIOMETER's center to A7 pin on ADC port of Arduino. One side of POT to to GND. Remaining side of POT to +5V. Connect ARef to +5V and Capacitor 0.1 microFarad between GND and +5V.


int main(void)
{
    //Set PORTH to OUTPUT
    DDRH |= _BV(DDH3);
    //Set the output port H pin 3 to 0 (LOW) state
    PORTH &= ~_BV(PORTH3);
  
    //Set PORTH to OUTPUT
    DDRH |= _BV(DDH4);
    //Set the output port H pin 4 to 0 (LOW) state
    PORTH &= ~_BV(PORTH4);

    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0);
    //Clear Interrupts
    cli(); 
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC -> prescaler to 128 - 125KHz sample rate @ 16MHz
    ADCSRB |= (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0); // Set ADC -> Free Running Mode
    ADMUX |= (0 << REFS1)| (1 << REFS0); // Set ADC reference to AVCC
    //ADMUX |= (0 << REFS1)| (0 << REFS0); //AREF, Internal VREF  turned off
    //ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit
    ADMUX |= (0 << ADLAR); // Default - right adjust ADC result to allow easy 8 bit
    ADCSRA |= (1 << ADEN); // Enable ADC
    ADCSRA |= (1 << ADIE); // Enable ADC Interrupt
    ADMUX |= (1 << MUX2)| (1 << MUX1) |(1 << MUX0); // using ADC7 pin 
    //Set Interrupts
    sei();
    while (1)
       {
             ADCSRA |= (1<<ADSC); // Start conversion
             //Instead of logic below for waiting for conversion we wait for ADC conversion complete interrupt
             //while (ADCSRA & (1<<ADSC)); // wait for conversion to complete
       }
    return 0;
}

ISR(ADC_vect){
   //ADCW stores the value
   unsigned int adc_value = ADCW;

   // max ADCW value is 2^10 = 1024 hence half of it. 
   if (adc_value < 512)
   {
      PORTH |= _BV(PORTH3); // Turn on LED1
      PORTH &= ~_BV(PORTH4); // Turn off LED2
   } else {
      PORTH &= ~_BV(PORTH3); // Turn off LED1
      PORTH |= _BV(PORTH4); // Turn on LED2
   }
}
