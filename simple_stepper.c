#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/*
Position where 1st coil is connected.
All other coils like 2nd,3rd and 4th
must be connected in sequence after the 1st.

For this configuration the connection is 
like:-

AVR 10 - PB4 (IN1 on ULN2003 board) -> Coil 1
AVR 11 - PB5 (IN2 on ULN2003 board) -> Coil 2
AVR 12 - PB6 (IN3 on ULN2003 board) -> Coil 3
AVR 13 - PB7 (IN4 on ULN2003 board) -> Coil 4

Don't connect port pins directly to coil,
use a driver like ULN2003A etc.

*/

int main(){

  /* 0 => clockwise */
  /* 1 => counter-clockwise */
  uint8_t dir = 1;

  /* set pin 7 of PORTB for output*/
  DDRB |= _BV(DDB4);
  DDRB |= _BV(DDB5);
  DDRB |= _BV(DDB6);
  DDRB |= _BV(DDB7);
 
  /* set output to 0 */
  PORTB &= ~_BV(PORTB4);
  PORTB &= ~_BV(PORTB5);
  PORTB &= ~_BV(PORTB6);
  PORTB &= ~_BV(PORTB7);

  int8_t step=0; 

while(1){
 switch(step){ 
   case 0: 
     PORTB &= ~_BV(PORTB4);
     PORTB &= ~_BV(PORTB5);
     PORTB &= ~_BV(PORTB6);
     PORTB |= _BV(PORTB7);
   break;  
   case 1: 
     PORTB &= ~_BV(PORTB4);
     PORTB &= ~_BV(PORTB5);
     PORTB |= _BV(PORTB6);
     PORTB |= _BV(PORTB7);
   break;  
   case 2: 
     PORTB &= ~_BV(PORTB4);
     PORTB &= ~_BV(PORTB5);
     PORTB |= _BV(PORTB6);
     PORTB &= ~_BV(PORTB7);
   break;  
   case 3: 
     PORTB &= ~_BV(PORTB4);
     PORTB |= _BV(PORTB5);
     PORTB |= _BV(PORTB6);
     PORTB &= ~_BV(PORTB7);
   break;  
   case 4: 
     PORTB &= ~_BV(PORTB4);
     PORTB |= _BV(PORTB5);
     PORTB &= ~_BV(PORTB6);
     PORTB &= ~_BV(PORTB7);
   break;  
   case 5: 
     PORTB |= _BV(PORTB4);
     PORTB |= _BV(PORTB5);
     PORTB &= ~_BV(PORTB6);
     PORTB &= ~_BV(PORTB7);
   break;  
     case 6: 
     PORTB |= _BV(PORTB4);
     PORTB &= ~_BV(PORTB5);
     PORTB &= ~_BV(PORTB6);
     PORTB &= ~_BV(PORTB7);
   break;  
   case 7: 
     PORTB |= _BV(PORTB4);
     PORTB &= ~_BV(PORTB5);
     PORTB &= ~_BV(PORTB6);
     PORTB |= _BV(PORTB7);
   break;  
   default: 
     PORTB &= ~_BV(PORTB7);
     PORTB &= ~_BV(PORTB6);
     PORTB &= ~_BV(PORTB5);
     PORTB &= ~_BV(PORTB4);
   break;  
 }
 if(dir > 0){ 
  step++;
 }else{
  step--;
 }
 if(step>7){ 
   step=0; 
 } 
 if(step<0){ 
   step=7; 
 } 
 _delay_ms(5);
 }
  return 0;
}

