//(c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
//Inspired and modified code https://balau82.wordpress.com/2011/03/29/programming-arduino-uno-in-pure-c/
//This is to blink the Elegoo onboard LED

#include <avr/io.h>
#include <util/delay.h>
 
#define BLINK_DELAY_MS 1000
 
int main (void)
{
 /* set pin 7 of PORTB for output*/
 DDRB |= _BV(DDB7);
 
 while(1) {
  /* set pin 7 high to turn led on */
  PORTB |= _BV(PORTB7);
  _delay_ms(BLINK_DELAY_MS);
 
  /* set pin 7 low to turn led off */
  PORTB &= ~_BV(PORTB7);
  _delay_ms(BLINK_DELAY_MS);
 }
}
