
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

//Connect VCC to 330 Ohms to +ve end of LED and -ve end of LED to pin 5 of Arduino
//Connect VCC to 330 Ohms to +ve end of LED and -ve end of LED to pin 4 of Arduino
//Connect 10K Ohms POTENTIOMETER's center to A7 pin on ADC port of Arduino. One side of POT to to GND. Remaining side of POT to +5V. Connect ARef to +5V and Capacitor 0.1 microFarad between GND and +5V.
//Connect pin 6 on Arduino to orange wire on servo, red wire on servo to +5V and brown wire on servo to GND.

/*
From Atmega2560 datasheet

freq OCnx = F_CPU / (PRESCALAR * (1 + TOP))
where TOP = ICRn in our case for mode 14 fast PWM

e.g. to create 50 Hz pulse as required by SG90 servo motor
50 = 16 MHz / ( 64 * (1 + ICRn))
(16 MHz / ( 50 * 64 )) - 1 = ICRn
ICRn = 4999
*/

static unsigned long positions[8] = {150, 200, 250, 312, 375, 438, 500, 562};
static unsigned long division(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor/2)) / divisor;
}


int main(void)
{
    //SG90 servo needs 50 Hz or 20 milliseconds pulse
    /*

	Weight: 9 g 
	Dimension: 22.2 x 11.8 x 31 mm approx. 
	Stall torque: 1.8 kgf-cm 
	Operating speed: 0.1 s/60 degree 
	Operating voltage: 4.8 V (~5V) 
	Dead band width: 10 μs 
	Temperature range: 0 ºC – 55 ºC 

        Leads: PWM (Orange), Vcc (Red), Ground (Brown)
        PWM Period: 20 milliseconds
        duty cycle varies between 1 and 2 ms
        Position -90 (1 milliseconds/20 milliseconds -> 5% duty cycle) - left position
        Position 0 (1.5 milliseconds/20 milliseconds -> 7.5% duty cycle)  - middle position
        Position +90 (2 milliseconds/20 milliseconds -> 10% duty cycle) - right position

    */

    unsigned long timer_frequency;
    unsigned long desired_frequency = 50; // 50 Hz
    const unsigned long prescalar = 64;

    //Set PORTE to OUTPUT LED
    DDRE |= _BV(DDE3);
    //Set the output port E pin 3 to 0 (LOW) state
    PORTE &= ~_BV(PORTE3);
  
    //Set PORTG to OUTPUT LED
    DDRG |= _BV(DDG5);
    //Set the output port G pin 5 to 0 (LOW) state
    PORTG &= ~_BV(PORTG5);

    // Initial PORT 
  
    //Set PORTH to OUTPUT
    DDRH |= _BV(DDH3);
    //Set the output port H pin 3 to 0 (LOW) state
    PORTH &= ~_BV(PORTH3);

    //Set up ADC
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

    timer_frequency = division(F_CPU, prescalar);
    ICR4 = division(timer_frequency, desired_frequency) - 1;

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
    while (1)
       {
             ADCSRA |= (1<<ADSC); // Start conversion
             //Instead of logic below for waiting for conversion we wait for ADC conversion complete interrupt
             //while (ADCSRA & (1<<ADSC)); // wait for conversion to complete
       }
   return 0;
}

ISR(ADC_vect){
   //Clear interrupt
   cli();

   //ADCW stores the value
   unsigned int adc_value = ADCW;

   // max ADCW value is 2^10 = 1024 hence half of it. 
   if (adc_value < 512)
   {
      PORTE |= _BV(PORTE3); // Turn on LED1
      PORTG &= ~_BV(PORTG5); // Turn off LED2
   } else {
      PORTE &= ~_BV(PORTE3); // Turn off LED1
      PORTG |= _BV(PORTG5); // Turn on LED2
   }
   // We vary the duty cycle between 3% and 11% @5V -> To move from 0 degrees to 180 degrees
   if(adc_value > 0 && adc_value <= 128){
            OCR4A = positions[0];
	   _delay_ms(500);
   } else if(adc_value > 128 && adc_value <= 256 ){
            OCR4A = positions[1];
	   _delay_ms(500);
   } else if(adc_value > 256 && adc_value <= 384 ){
            OCR4A = positions[2];
	   _delay_ms(500);
   } else if(adc_value > 384 && adc_value <= 512 ){
            OCR4A = positions[3];
	   _delay_ms(500);
   } else if(adc_value > 512 && adc_value <= 640 ){
            OCR4A = positions[4];
	   _delay_ms(500);
   } else if(adc_value > 640 && adc_value <= 768 ){
            OCR4A = positions[5];
	   _delay_ms(500);
   } else if(adc_value > 768 && adc_value <= 896 ){
            OCR4A = positions[6];
	   _delay_ms(500);

   } else if(adc_value > 896 && adc_value <= 1024 ){
            OCR4A = positions[7];
	   _delay_ms(500);
   } else {
            OCR4A = positions[0];
	   _delay_ms(500);
   }
   //Set interrupt
   sei();
}
