#include <avr/io.h>
#include <util/delay.h>

//Connect PWM GND pin to 330 Ohms resistor. Connect LED with -ve towards resistor and +ve towards PWM pin 6 PWM on Arduino

int main(void)
{
  unsigned char duty_cycle;

  // Initial PORT 
  
  //Set PORTH to OUTPUT
  DDRH |= _BV(DDH3);
  //Set the output port H pin 3 to 0 (LOW) state
  PORTH &= ~_BV(PORTH3);

  //This pin PORTH3 is tied to 16 bit timer TC4 and OCR4A is the register to use it

   // Set the TIMER4 16 bit Counter -> This will be TOP value
   TCNT4H = 0;
   TCNT4L = 0XFF;

   // Set the TIMER4 PWM Duty Cycle on OCR4AH and OCR4AL
   // Always use half of the TOP value (PWM Duty Cycle ~ 50%)
   duty_cycle=0xFF / 2;

   OCR4AH=(duty_cycle >> 8 ) & 0x00FF;
   OCR4AL=duty_cycle;

  // Initial TIMER4 Fast PWM
  // Fast PWM Frequency = fclk / (N * TOP), Where N is the Prescaler
  // f_PWM = 16 MHz / (1024 * TOP) = X Hz
  TCCR4A |= 0<<WGM41 | 1<<WGM40; // Fast PWM 8 Bit
  TCCR4B |= 1<<WGM42; // Fast PWM 8 Bit
  // Clear OCA4 on Compare Match, Set OC4A at BOTTOM 
  TCCR4A |= 1<<COM4A1 | 0<<COM4A0;
  // Used 1024 Prescaler
  TCCR4B |= 1<<CS12 | 0<<CS11 | 1<<CS10;  //Divide by 1024 (We are using 16 MHz external Crystal)  

  duty_cycle=0;	// Initial Duty Cycle for Channel A

  for(;;) {            // Loop Forever
    //Gradually increase LED brightness
    while(duty_cycle < 0xFF) {
       duty_cycle += 1;
       OCR4AH=(duty_cycle >> 8 ) & 0x00FF;
       OCR4AL=duty_cycle;
       _delay_ms(10);
    }
    //Gradually decrease LED brightness
    while(duty_cycle > 0x00) {
       duty_cycle -= 1;
       OCR4AH=(duty_cycle >> 8 ) & 0x00FF;
       OCR4AL=duty_cycle;
       _delay_ms(10);
    }
  }
  return 0;	        // Standard Return Code
}
