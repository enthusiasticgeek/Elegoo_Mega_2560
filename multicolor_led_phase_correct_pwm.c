//Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Connect PWM GND pin to 330 Ohms resistor. Connect RED LED terminal with -ve towards resistor and +ve towards PWM pin 6 PWM on Arduino
//Connect PWM GND pin to 330 Ohms resistor. Connect GREEN LED terminal with -ve towards resistor and +ve towards PWM pin 5 PWM on Arduino
//Connect PWM GND pin to 330 Ohms resistor. Connect BLUE LED terminal with -ve towards resistor and +ve towards PWM pin 2 PWM on Arduino
//Connect LED GND to GND of the circuit.

int main(void)
{
  unsigned char duty_cycle_blue;
  unsigned char duty_cycle_green;
  unsigned char duty_cycle_red;

  // Initial PORT 
  
  //This pin PORTH3 is tied to 16 bit timer TC4 and OCR4A is the register to use it
  //Set PORTH to OUTPUT
  DDRH |= _BV(DDH3);
  //Set the output port H pin 3 to 0 (LOW) state
  PORTH &= ~_BV(PORTH3);
  
  //This pin PORTE3 is tied to 16 bit timer TC3 and OCR3A is the register to use it
  //Set PORTE to OUTPUT
  DDRE |= _BV(DDE3);
  //Set the output port E pin 3 to 0 (LOW) state
  PORTE &= ~_BV(PORTE3);
  
  //This pin PORTE5 is tied to 16 bit timer TC3 and OCR3C is the register to use it
  //Set PORTE to OUTPUT
  DDRE |= _BV(DDE5);
  //Set the output port E pin 5 to 0 (LOW) state
  PORTE &= ~_BV(PORTE5);

   //Disable Interrupts
   cli(); 

  //############################################################
   // Set the TIMER4 16 bit Counter -> This will be TOP value in some modes of PWM
   TCNT4H = 0;
   TCNT4L = 0XFF;

   // Set the TIMER3 16 bit Counter -> This will be TOP value in some modes of PWM
   TCNT3H = 0;
   TCNT3L = 0XFF;
 
  //########################### BLUE ##########################

   // Set the TIMER4 PWM Duty Cycle on OCR4AH and OCR4AL
   // Always use half of the TOP value (PWM Duty Cycle ~ 50%)
   duty_cycle_blue=0xFF / 2;

   OCR4AH=(duty_cycle_blue >> 8 ) & 0x00FF;
   OCR4AL=duty_cycle_blue;

  // Initial TIMER4 Phase correct PWM
  // Phase correct PWM Frequency = fclk / (N * TOP), Where N is the Prescaler
  // f_PWM = 16 MHz / (1024 * TOP) = X Hz
  TCCR4A |= 0<<WGM41 | 1<<WGM40; // Phase correct 8-bit PWM with 16 Bit timer
  TCCR4B |= 0<<WGM42; // Phase correct 8-bit PWM with 16 Bit timer
  //Set OC4A on compare match when up-counting
  //Clear OC4A on compare match when downcounting
  TCCR4A |= 1<<COM4A1 | 1<<COM4A0;
  // Used 1024 Prescaler
  TCCR4B |= 1<<CS12 | 0<<CS11 | 1<<CS10;  //Divide by 1024 (We are using 16 MHz external Crystal)  

  //############################################################
  //########################### GREEN ##########################

   // Set the TIMER3 PWM Duty Cycle on OCR3AH and OCR3AL
   // Always use half of the TOP value (PWM Duty Cycle ~ 25%)
   duty_cycle_green=0xFF / 4;

   OCR3AH=(duty_cycle_green >> 8 ) & 0x00FF;
   OCR3AL=duty_cycle_green;

  // Initial TIMER3 Phase correct PWM
  // Phase correct PWM Frequency = fclk / (N * TOP), Where N is the Prescaler
  // f_PWM = 16 MHz / (1024 * TOP) = X Hz
  TCCR3A |= 0<<WGM31 | 1<<WGM30; // Phase correct 8-bit PWM with 16 Bit timer
  TCCR3B |= 0<<WGM32; // Phase correct 8-bit PWM with 16 Bit timer
  //Set OC3A on compare match when up-counting
  //Clear OC3A on compare match when downcounting
  TCCR3A |= 1<<COM3A1 | 1<<COM3A0;
  // Used 1024 Prescaler
  TCCR3B |= 1<<CS12 | 0<<CS11 | 1<<CS10;  //Divide by 1024 (We are using 16 MHz external Crystal)  

  //############################################################
  //########################### RED ##########################

   // Set the TIMER3 PWM Duty Cycle on OCR3BH and OCR3BL
   // Always use half of the TOP value (PWM Duty Cycle ~ 25%)
   duty_cycle_red=0xFF;

   OCR3CH=(duty_cycle_red >> 8 ) & 0x00FF;
   OCR3CL=duty_cycle_red;

  // Initial TIMER3 Phase correct PWM
  // Phase correct PWM Frequency = fclk / (N * TOP), Where N is the Prescaler
  // f_PWM = 16 MHz / (1024 * TOP) = X Hz
  TCCR3A |= 0<<WGM31 | 1<<WGM30; // Phase correct 8-bit PWM with 16 Bit timer
  TCCR3B |= 0<<WGM32; // Phase correct 8-bit PWM with 16 Bit timer
  //Set OC3C on compare match when up-counting
  //Clear OC3C on compare match when downcounting
  TCCR3A |= 1<<COM3C1 | 1<<COM3C0;
  // Used 1024 Prescaler
  TCCR3B |= 1<<CS12 | 0<<CS11 | 1<<CS10;  //Divide by 1024 (We are using 16 MHz external Crystal)  

  //############################################################

  //Enable Interrupts
  sei();
  //duty_cycle_blue=0;	// Initial Duty Cycle for Channel A

  for(;;) {            // Loop Forever
    //Gradually increase BLUE LED brightness
    while(duty_cycle_blue < 0xFF) {
       duty_cycle_blue += 1;
       OCR4AH=(duty_cycle_blue >> 8 ) & 0x00FF;
       OCR4AL=duty_cycle_blue;
       _delay_ms(10);
    }
    //Gradually decrease BLUE LED brightness
    while(duty_cycle_blue > 0x00) {
       duty_cycle_blue -= 1;
       OCR4AH=(duty_cycle_blue >> 8 ) & 0x00FF;
       OCR4AL=duty_cycle_blue;
       _delay_ms(10);
    }

    //Gradually increase GREEN LED brightness
    while(duty_cycle_green < 0xFF) {
       duty_cycle_green += 1;
       OCR3AH=(duty_cycle_green >> 8 ) & 0x00FF;
       OCR3AL=duty_cycle_green;
       _delay_ms(10);
    }
    //Gradually decrease GREEN LED brightness
    while(duty_cycle_green > 0x00) {
       duty_cycle_green -= 1;
       OCR3AH=(duty_cycle_green >> 8 ) & 0x00FF;
       OCR3AL=duty_cycle_green;
       _delay_ms(10);
    }

    //Gradually increase RED LED brightness
    while(duty_cycle_red < 0xFF) {
       duty_cycle_red += 1;
       OCR3CH=(duty_cycle_red >> 8 ) & 0x00FF;
       OCR3CL=duty_cycle_red;
       _delay_ms(10);
    }
    //Gradually decrease RED LED brightness
    while(duty_cycle_red > 0x00) {
       duty_cycle_red -= 1;
       OCR3CH=(duty_cycle_red >> 8 ) & 0x00FF;
       OCR3CL=duty_cycle_red;
       _delay_ms(10);
    }

  }
  return 0;	        // Standard Return Code
}
