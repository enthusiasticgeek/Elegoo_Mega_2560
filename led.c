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
