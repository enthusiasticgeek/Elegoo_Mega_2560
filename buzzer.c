// Modified from https://balau82.wordpress.com/2014/10/15/using-a-buzzer-with-arduino-in-pure-c/

#include <avr/io.h>
#include <util/delay.h>

//Sound Waveform frequency = Crystal frequency / 2*Prescalar*(1 + value in OCRnX)
//Hence,  (Crystal frequency / (Sound Waveform frequency*2*Prescalar)) - 1 = value in OCRnX

//Sound Waveform frequencies for Piano https://en.wikipedia.org/wiki/Piano_key_frequencies

//e.g. To generate 440 Hz with Atmega running 16 MHz clock and prescalar value 256 and OCR0A value 70
//Sound Waveform frequency = 16 MHz / 2*256*(1 + 70) = 440.140 Hz

//To find OCR0A given the Sound waveform frequency
//Hence, 440.140 / 16 MHz = 1 / 2*256*(1 + OCR0A)
//Hence, 16 MHz / ( 440.140 * 2 * 256) = 1 + OCR0A
//Hence, 71 = 1 + OCR0A
//Hence, OCR0A = 70
 
// Round division explained here: http://www.avrfreaks.net/forum/division-attiny 

static unsigned long division_round(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor/2)) / divisor;
           
}
 
static void update_waveform_frequency(unsigned long hz, unsigned long timer_frequency)
{
    OCR0A = division_round(timer_frequency, hz*2) - 1;
    TCCR0A = 
          _BV(COM0A0) // toggle
        | _BV(WGM01); // CTC mode
}
 
int main(void)
{
    unsigned long timer_frequency;
 
    DDRD |= _BV(D6);
    //Set prescalar 256
    TCCR0B = _BV(CS02);
    timer_frequency = div_round(F_CPU, 256);
 
    while(1)
    {
        update_waveform_frequency(440, timer_frequency);
        _delay_ms(200);
        update_waveform_frequency(880, timer_frequency);
        _delay_ms(200);
    }
    return 0;
}
