//Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
// Tested with WINGONEER USB 2.0 to TTL UART 6PIN CP2102 Module Serial Converter
// Connect RX1 pin of the AVR to TX pin of the serial converter 
// Connect TX1 pin of the AVR to RX pin of the serial converter 
// Connect GND pin of the AVR to GND pin of the serial converter 
// Open terminal. if /dev/ttyUSB0 is connected as serial device
// 'sudo minicom -s configuration ttyUSB0'
// set baudrate 115200 8N1 (no parity, 8 bit data, 1 stop bit). Then press CTRL+a. Then press x.
// 'sudo minicom configuration ttyUSB0' and press ENTER 

/*Very Important - change F_CPU to match target clock 
  Note: default AVR clock is 16MHz external crystal
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
#define USART_BAUDRATE 115200   
//#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define BAUD_PRESCALE (unsigned int)( F_CPU / (16.0 * USART_BAUDRATE) - 0.5 )

unsigned char value;  
/* This variable is volatile so both main and RX interrupt can use it.
It could also be a uint8_t type */

/* Interrupt Service Routine for Receive Complete 
NOTE: vector name changes with different AVRs see AVRStudio -
Help - AVR-Libc reference - Library Reference - <avr/interrupt.h>: Interrupts
for vector names other than USART_RXC_vect for ATmega32 */

ISR(USART1_RX_vect){
   cli();                   // clear interrupt	
   value = UDR1;            // read UART register into value
   PORTB = ~value;          // output inverted value on LEDs (0=on)
   sei();                   // enable interrupts	
}

void USART_Init(void){
   // Set baud rate
   /* Load upper 8-bits into the high byte of the UBRR register
      Default frame format is 8 data bits, no parity, 1 stop bit
      to change use UCSRC, see AVR datasheet */ 
   // Enable receiver and transmitter and receive complete interrupt 
   UCSR1B = (1<<TXEN1)|(1<<RXEN1)|(0<<UCSZ12)|(1<<RXCIE1);//|(1<<TXCIE1);
   //8-bit data, 1 stop bit, Aynchronous USART, no parity	
   UCSR1C = (1<<UCSZ11)|(1<<UCSZ10)|(0<<USBS1)|(0<<UMSEL11)|(0<<UMSEL10)|(0<<UPM11)|(0<<UPM10);
   UBRR1H = (uint8_t) (BAUD_PRESCALE >> 8); 
   UBRR1L = (uint8_t) (BAUD_PRESCALE);// Load lower 8-bits into the low byte of the UBRR register
}

void USART_SendByte(unsigned char u8Data){
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
   /* set pin 7 of PORTB for output*/
   DDRB |= _BV(DDB7);
}

int main(void){
   cli();
   USART_Init();   // Initialise USART
   sei();          // enable all interrupts
   Led_init();     // init LEDs for testing
   value = 'A';    // 0x41;    
   for(;;){                  // Repeat indefinitely
     PORTB = PORTB ^ _BV(PORTB7);// toggle built in LED on TX
     USART_SendByte(value);  // send value 
     _delay_ms(550);         // delay just to stop minicom screen cluttering    
   }
}

