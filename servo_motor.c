//(c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

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
 
    while(1)
    {

        // We vary the duty cycle between 3% and 11% @5V -> To move from 0 degrees to 180 degrees
        //0 degrees
          OCR4A = 150;
	_delay_ms(500);
 
          OCR4A = 200;
	_delay_ms(500);
       
         OCR4A = 250;
	_delay_ms(500);

         OCR4A = 312;
	_delay_ms(500);
 
        //~ 90 degrees

         OCR4A = 375;
	_delay_ms(500);

         OCR4A = 438;
	_delay_ms(500);

         OCR4A = 500;
	_delay_ms(500);

        //180 degrees
         OCR4A = 562;
	_delay_ms(500);
        
    }
    return 0;
}
