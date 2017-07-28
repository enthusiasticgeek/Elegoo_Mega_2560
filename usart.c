//#define F_CPU 16000000UL  // 16 MHz

/*Very Important - change F_CPU to match target clock 
  Note: default AVR CLKSEL is 1MHz internal RC
  This program transmits continously on USART. Interrupt is used for 
	Receive charactor, which is then transmitted instead. LEDs are used 
	as a test. Normal RX routine is included but not used.
  Change USART_BAUDRATE constant to change Baud Rate
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 16000000UL                    // set the CPU clock
#endif

// Define baud rate
#define USART_BAUDRATE 38400   
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

volatile unsigned char value;  
/* This variable is volatile so both main and RX interrupt can use it.
It could also be a uint8_t type */

/* Interrupt Service Routine for Receive Complete 
NOTE: vector name changes with different AVRs see AVRStudio -
Help - AVR-Libc reference - Library Reference - <avr/interrupt.h>: Interrupts
for vector names other than USART_RXC_vect for ATmega32 */

ISR(USART1_RX_vect){
   value = UDR1;            //read UART register into value
   PORTB = ~value;          // output inverted value on LEDs (0=on)
}

void USART_Init(void){
   // Set baud rate
   UBRR1L = BAUD_PRESCALE;// Load lower 8-bits into the low byte of the UBRR register
   UBRR1H = (BAUD_PRESCALE >> 8); 
   /* Load upper 8-bits into the high byte of the UBRR register
      Default frame format is 8 data bits, no parity, 1 stop bit
      to change use UCSRC, see AVR datasheet*/ 
   // Enable receiver and transmitter and receive complete interrupt 
   UCSR1B = ((1<<TXEN1)|(1<<RXEN1) | (1<<RXCIE1));
   UCSR1C = (1<<USBS1)|(1<<UCSZ11)|(1<<UCSZ10);
}

void USART_SendByte(uint8_t u8Data){
  // Wait until last byte has been transmitted
  while((UCSR1A &(1<<UDRE1)) == 0);
  // Transmit data
  UDR1 = u8Data;
}

// Wait until a byte has been received and return received data 
uint8_t USART_ReceiveByte(){
  while((UCSR1A &(1<<RXC1)) == 0);
  return UDR1;
}

void Led_init(void){
   //outputs, all off
   DDRB =0xFF;       
   PORTB = 0xFF;        
}

int main(void){
   USART_Init();   // Initialise USART
   sei();          // enable all interrupts
   Led_init();     // init LEDs for testing
   value = 'A';    // 0x41;    
   PORTB = ~value; // 0 = LED on
   
   for(;;){                  // Repeat indefinitely
     USART_SendByte(value);  // send value 
     _delay_ms(250);         // delay just to stop minicom screen cluttering    
   }
}
