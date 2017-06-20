//(c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
#include <avr/io.h>
#include <util/delay.h>

//Connect pin 13 on Arduino to +ve side Active buzzer and GND to -ve pin on Active buzzer.

/*
From Atmega2560 datasheet

freq OCnx = F_CPU / (2 * PRESCALAR * (1 + OCRnx))
e.g. to create 880 Hz sound
880 = 16 MHz / ( 2 * 256 * (1 + OCRnx))
(16 MHz / ( 880 * 2 * 256 )) - 1 = OCRnx
OCRnx = 34.51 =~ 35
*/


static unsigned long division(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor/2)) / divisor;
}

int main(void)
{
    unsigned long timer_frequency;
    const unsigned long prescalar = 256;

    //Clear Interrupts
    cli(); 

    DDRB |= _BV(DDB7);
    timer_frequency = division(F_CPU, prescalar);
    OCR0A = division(timer_frequency, prescalar*2) - 1;
    //Clear OC0A on compare match, set OC0A at BOTTOM (non-inverting mode)
    TCCR0A |= 1<<COM0A1 | 0<<COM0A0;
    TCCR0A =  _BV(WGM01) | _BV(WGM00); // 8-bit FAST PWM
    TCCR0B = _BV(CS02); // 256 prescalar

    //Set Interrupts
    sei();
 
    while(1)
    {

    }
    return 0;
}
