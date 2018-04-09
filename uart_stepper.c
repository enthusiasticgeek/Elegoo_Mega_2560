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
ELEGOO ANALOG GND - (GND on ULN2003 board) -> This is usually last pin on the side away from resistors depending on the ULN2003 board layout
ELEGOO ANALOG +5V - (+5V on ULN2003 board) -> This is usually 2nd pin on the side away from resistors depending on the ULN2003 board layout
Don't connect port pins directly to coil,
use a driver like ULN2003A etc.

*/
// Tested with WINGONEER USB 2.0 to TTL UART 6PIN CP2102 Module Serial Converter
// Connect RX1 pin of the AVR to TX pin of the serial converter
// Connect TX1 pin of the AVR to RX pin of the serial converter
// Connect GND pin of the AVR to GND pin of the serial converter
// Open terminal. if /dev/ttyUSB0 is connected as serial device
// 'sudo minicom -s configuration ttyUSB0'
// set baudrate 115200 8N1 (no parity, 8 bit data, 1 stop bit). Then press CTRL+a. Then press x.
// 'sudo minicom configuration ttyUSB0' and press ENTER few times and follow the instructions.
// Now send commands
// To rotate clockwise fast e.g. send command 'CW,F'
// To stop e.g. send command 'STP'
// To rotate counter clockwise slow send command  'CCW,S'
// To rotate clockwise medium speed e.g. 'CW,M'

/*Very Important - change F_CPU to match target clock
  Note: default AVR clock is 16MHz external crystal
  This program transmits continously on USART. Interrupt is used for
	Receive charactor, which is then transmitted instead. LEDs are used
	as a test. Normal RX routine is included but not used.
  Change USART_BAUDRATE constant to change Baud Rate
*/



typedef enum {
    STOP=-1,
    CLOCKWISE=0,
    COUNTER_CLOCKWISE=1,
} DIRECTION;

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
int send_prompt=1;
/* 0 => clockwise */
/* 1 => counter-clockwise */
volatile int8_t dir = STOP;
char speed[2]= {};
int32_t total_steps=-1;
char checksum_received_string[3]= {};
volatile bool checksum_match = false;
char checksum_calc_string[255]= {};

unsigned short crc16_ccitt(const unsigned char* data_p, unsigned char length);
unsigned short checksum_received;
ISR(USART1_RX_vect)
{
    step_counter=0;
    //initialize char array with the terminating character
    int k;
    for(k=0; k < strlen(checksum_calc_string); k++) {
        checksum_calc_string[k]='\0';
    }
    buffer[i]=UDR1;         //Read USART data register
    if(buffer[i++]=='\r')   //check for carriage return terminator and increment buffer index
    {
        //Make a copy
        memset(buffer_send, '\0', sizeof(buffer_send));
        strncpy(buffer_send, buffer, sizeof(buffer));

        //Now parse the string
        //j represent fields separated by comma delimiter.
        uint8_t j=0;
        const char delimiter[2] = ",";
        char *token=NULL;
        /* get the first token */
        token = strtok(buffer, delimiter);
        if((j==0) && (token != NULL)) {
            if(strncmp(token,"CW",2)==0) {
                dir = CLOCKWISE;
            } else if (strncmp(token,"CCW",3)==0) {
                dir = COUNTER_CLOCKWISE;
            } else if (strncmp(token,"STP",3)==0) {
                dir = STOP;
            }
            strncat(checksum_calc_string,token,strlen(token));
        }
        /* walk through other tokens */
        while( token != NULL ) {
            j++;
            token = strtok(NULL, delimiter);
            if((j==1) && (token != NULL)) {
                /*set speed*/
                strncpy(speed,token,1);
                strncat(checksum_calc_string,token,strlen(token));
            }
            if((j==2) && (token != NULL)) {
                /*set steps*/
                /*
                errno = 0;
                long int result = strtol (buffer, &token, sizeof(token));
                if (result == LONG_MIN && errno != 0)
                {
                   // Underflow.
                }
                if (result == LONG_MAX && errno != 0)
                {
                   // Overflow.
                }
                */
                total_steps = atoi(token);
                strncat(checksum_calc_string,token,strlen(token));
            }
            if((j==3) && (token != NULL)) {
                //checksum
                strncpy(checksum_received_string,token,2);
                checksum_received = ((checksum_received_string[1] << 8) & 0xFF00) | (checksum_received_string[0] & 0xFF);
                unsigned short checksum_calculated = crc16_ccitt((const unsigned char*)checksum_calc_string,strlen(checksum_calc_string));
                if(checksum_calculated == checksum_received) {
                    checksum_match = true;
                } else {
                    checksum_match = false;
                }
            }

        }

        // if terminator detected
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

unsigned char CheckSum(char *buffer)
{
    uint8_t i;
    unsigned char xor_1;
    unsigned long length = strlen(buffer);
    for (xor_1 = 0, i = 0; i < length; i++)
        xor_1 ^= (unsigned char)buffer[i];
    return xor_1;
}

unsigned short crc16_ccitt(const unsigned char* data_p, unsigned char length) {
    unsigned char x;
    unsigned short crc = 0xFFFF;
    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

int main() {

    /* 0 => clockwise */
    /* 1 => counter-clockwise */
    cli();
    USART_Init();   // Initialise USART
    sei();          // enable all interrupts
    uint8_t first_msg=0;


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

    while(1) {
//USART_putstring("\nHello\n\r");
        if(first_msg==0) {
            first_msg=1;
            USART_putstring("Write CW or CCW or STP to rotate the motor clockwise or counter clockwise or halt motion.\n\rExample: CW,F (CLOCKWISE FAST) or CCW,S (COUNTER-CLOCKWISE SLOW) or CW,M (CLOCKWISE MEDIUM) or STP (STOP)\n\rRepeat this process to control the stepper motor.\n\r");
        }
        if (StrRxFlag)
        {
            //Echo the string back
            USART_putstring(buffer_send);
            USART_SendByte('\n');           // Send linefeed
            USART_SendByte('O');           // Send linefeed
            USART_SendByte('K');           // Send linefeed
            USART_SendByte('\r');           // Send carriage return
            USART_SendByte('\n');           // Send linefeed
            StrRxFlag=0;                // Reset String received flag
            send_prompt=1;
        }
//send prompt
        if(send_prompt==1) {
            USART_SendByte('_');
            USART_SendByte('\r');           // Send linefeed
            send_prompt=0;
        }
//Stepper motor control logic
        switch(step) {
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
        if((dir == COUNTER_CLOCKWISE)&&(checksum_match==true)) {
            if(total_steps>0) {
                if(step_counter < total_steps)
                {
                    step_counter++;
                    step++;
                }
            }
            else {
                step++;
            }
        } else if ((dir == CLOCKWISE)&&(checksum_match==true)) {
            if(total_steps>0) {
                if(step_counter < total_steps)
                {
                    step_counter++;
                    step--;
                }
            }
            else {
                step--;
            }
        }
        if(step>7) {
            step=0;
        }
        if(step<0) {
            step=7;
        }
//Fast, slow or medium angular velocity rotation
        if((strncmp(speed,"F",1)==0)&&(checksum_match==true)) {
            _delay_ms(1);
        } else if((strncmp(speed,"M",1)==0)&&(checksum_match==true)) {
            _delay_ms(5);
        } else if((strncmp(speed,"S",1)==0)&&(checksum_match==true)) {
            _delay_ms(10);
        } else {
            _delay_ms(20);
        }

    }
    return 0;
}

