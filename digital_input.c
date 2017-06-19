//(c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
//Inspired and modified code https://balau82.wordpress.com/2011/03/29/programming-arduino-uno-in-pure-c/
//This is to blink the Elegoo onboard LED

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
   /* Digital Output Pin 13 LED on Arduino Mega 2560*/
   DDRB |= _BV(PB7);
   /* Digital Input Pin 6 on Arduino Mega 2560*/
   /* Pull up resistor 10K Ohms between 5V and one end push button switch; 
   connect the same end of push button switch to input pin 6
   and connect the other end of push button switch to GND*/
   DDRH &= ~(_BV(PH3));
   for(;;){
        // While Button is pressed, LED is on
        if (bit_is_clear(PINH, PH3)) {
        PORTB |= (1<<PB7); // put LED HIGH
        } else {
        PORTB &= ~(1<<PB7); // put LED LOW
        }
    }
return 0;
}



