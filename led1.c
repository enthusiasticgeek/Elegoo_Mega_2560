//Inspired from http://hackaday.com/2010/11/19/

#include <avr/io.h>
#include <avr/interrupt.h>
 
int main (void)
{
  //Setup the I/O for the LED
 /* set pin 7 of PORTB for output*/
 DDRB |= _BV(DDB7);
 /* set pin 7 high to turn led on */
 PORTB |= _BV(PORTB7);
 /* set pin 7 low to turn led off */
 //PORTB &= ~_BV(PORTB7);

 //Setup the clock
  cli();            //Disable global interrupts
  TCCR1B |= 1<<CS12 | 0<<CS11 | 1<<CS10;  //Divide by 1024
  //OCR1A = 15624;        //Count 15624 cycles for 1 second interrupt
  OCR1A = 7812;        //Count 7812 cycles for 0.5 second interrupt (LED being 0.5 sec ON and 0.5 sec OFF)
  TCCR1B |= 1<<WGM12;     //Put Timer/Counter1 in CTC mode
  TIMSK1 |= 1<<OCIE1A;        //enable timer compare interrupt
  sei();            //Enable global interrupts

 while(1) {
 }
}

ISR(TIMER1_COMPA_vect)      //Interrupt Service Routine
{
  PORTB ^= _BV(PORTB7);        //Use xor to toggle the LED
}
