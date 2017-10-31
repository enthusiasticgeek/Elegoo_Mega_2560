//Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
//This is to turn ON/OFF the Elegoo onboard LED when an input is applied
//Connect digital pin 6 to a push button switch's one end 
//while connecting GND to another end of push button switch (no need of external pull-up or pull-down resistors)

//This code can also be adapted to tilt switch

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
   /* Digital Output Pin 13 onboard LED on Arduino Mega 2560*/
   DDRB |= _BV(PB7);
   /* Digital Input Pin 6 on Arduino Mega 2560*/
   DDRH &= ~(_BV(PH3));
   /*Internal input pullup resistor high*/
   PORTH |= (1<<PH3); 
   /* IF above is not done then external Pull up resistor 10K Ohms between 5V and one end push button switch; 
   connect the same end of push button switch to input pin 6
   and connect the other end of push button switch to GND*/
   
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



