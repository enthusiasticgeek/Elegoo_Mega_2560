//Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
//http://embedds.com/programming-avr-i2c-interface/
//https://davidegironi.blogspot.com/2013/02/avr-atmega-mpu6050-gyroscope-and.html#.WuBXsNYh1hE

// Tested with WINGONEER USB 2.0 to TTL UART 6PIN CP2102 Module Serial Converter
// Connect RX1 pin of the AVR to TX pin of the serial converter
// Connect TX1 pin of the AVR to RX pin of the serial converter
// Connect GND pin of the AVR to GND pin of the serial converter
// Open terminal. if /dev/ttyUSB0 is connected as serial device
// 'sudo minicom -s configuration ttyUSB0'
// set baudrate 115200 8N1 (no parity, 8 bit data, 1 stop bit). Then press CTRL+a. Then press x.
// 'sudo minicom configuration ttyUSB0' and press ENTER few times and follow the instructions.
/*Very Important - change F_CPU to match target clock
  Note: default AVR clock is 16MHz external crystal
  This program transmits continously on USART. Interrupt is used for
	Receive charactor, which is then transmitted instead. LEDs are used
	as a test. Normal RX routine is included but not used.
  Change USART_BAUDRATE constant to change Baud Rate
*/

#ifndef F_CPU
#define F_CPU 16000000UL                    // set the CPU clock
#endif

// Define baud rate
#define USART_BAUDRATE 115200
//#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define BAUD_PRESCALE (unsigned int)( F_CPU / (16.0 * USART_BAUDRATE) - 0.5 )

//unsigned char value;
/* This variable is volatile so both main and RX interrupt can use it.
It could also be a uint8_t type */

/* Interrupt Service Routine for Receive Complete
NOTE: vector name changes with different AVRs see AVRStudio -
Help - AVR-Libc reference - Library Reference - <avr/interrupt.h>: Interrupts
for vector names other than USART_RXC_vect for ATmega32 */

volatile int32_t step_counter=0;
volatile int i=0;
char buffer[255]= {};
char buffer_send[255]= {};
volatile uint8_t StrRxFlag=0;
ISR(USART1_RX_vect)
{
    buffer[i]=UDR1;         //Read USART data register
    if(buffer[i++]=='\r')   //check for carriage return terminator and increment buffer index
    {
        //Make a copy
        memset(buffer_send, '\0', sizeof(buffer_send));
        //strncpy(buffer_send, buffer, sizeof(buffer-1));


        // if terminator detected
        strncat(buffer_send,"GO",2);

        //strncpy(buffer_send, buffer, sizeof(buffer_send));
        //snprintf(buffer_send, sizeof(buffer_send),"%d,%s,%s\n\r",dir,speed,total_steps);
        StrRxFlag=1;        //Set String received flag
        buffer[i-1]=0x00;   //Set string terminator to 0x00
        i=0;                //Reset buffer index
    }
}

void USART_Init(void) {
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

void USART_SendByte(unsigned char u8Data) {
    // Wait until last byte has been transmitted
    while((UCSR1A &(1<<UDRE1)) == 0);
    // Transmit data
    UDR1 = u8Data;
}

// Wait until a byte has been received and return received data
uint8_t USART_ReceiveByte() {
    while((UCSR1A &(1<<RXC1)) == 0);
    return UDR1;
}

//Alternate string TX function
void USART_putstring(char* StringPtr) {
    while(*StringPtr != 0x00) {
        USART_SendByte(*StringPtr);
        StringPtr++;
    }
}


int main() {

    cli();
    USART_Init();   // Initialise USART
    sei();          // enable all interrupts

    while(1) {
        if (StrRxFlag)
        {
            //Echo the string back
            USART_putstring(buffer_send);
            USART_SendByte('\r');           // Send carriage return
            USART_SendByte('\n');           // Send linefeed
            StrRxFlag=0;                // Reset String received flag
        }
    }
    return 0;
}

