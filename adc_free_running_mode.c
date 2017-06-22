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

static unsigned long positions[8] = {150, 200, 250, 312, 375, 438, 500, 562};
static unsigned long division(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor/2)) / divisor;
}

int main(void)
{
    unsigned long timer_frequency;
    unsigned long desired_frequency = 50; // 50 Hz
    const unsigned long prescalar = 64;
    //unsigned long duty_cycle_start;
    //unsigned long duty_cycle;
    //unsigned long duty_cycle_end;

    // Initial PORT 
  
    //Set PORTH to OUTPUT
    DDRH |= _BV(DDH3);
    //Set the output port H pin 3 to 0 (LOW) state
    PORTH &= ~_BV(PORTH3);

    //Clear Interrupts
    cli(); 

    timer_frequency = division(F_CPU, prescalar);
    ICR4 = division(timer_frequency, desired_frequency) - 1;

    //unsigned long icr4_value = division(timer_frequency, desired_frequency) - 1;
    // use 3% duty cycle to begin
    //duty_cycle_start=division(icr4_value, 300); //dividend 3*100
    // use 11% duty cycle to end
    //duty_cycle_end=division(icr4_value, 1100); //dividend 11*100

    //initialize duty cycle
    //duty_cycle = duty_cycle_start;

    // Initial TIMER4 Fast PWM
    // Fast PWM Frequency = fclk / (N * TOP), Where N is the Prescaler
    TCCR4A |= 1<<WGM41 | 0<<WGM40; // Fast PWM - Mode 14 with 16 Bit timer
    TCCR4B |= 1<<WGM43 | 1<<WGM42; // Fast PWM - Mode 14 with 16 Bit timer
    //Clear OC4A on compare match, set OC4A at BOTTOM (non-inverting mode)
    TCCR4A |= 1<<COM4A1 | 0<<COM4A0;
    // Used 64 Prescaler
    TCCR4B |= 0<<CS12 | 1<<CS11 | 1<<CS10;  //Divide by 64 (We are using 16 MHz external Crystal)  
    
    //Set Interrupts
    sei();
 
    unsigned int i = 0;
    while(1)
    {

    }
    return 0;
}
