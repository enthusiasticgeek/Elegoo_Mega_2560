//Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifndef BLINK_DELAY_MS_SLOW
#define BLINK_DELAY_MS_SLOW 500
#endif

#ifndef BLINK_DELAY_MS_FAST
#define BLINK_DELAY_MS_FAST 250
#endif

#ifndef UNKNOWN_POSITION
#define UNKNOWN_POSITION 255
#endif

#ifndef UNKNOWN_FUNCTION
#define UNKNOWN_FUNCTION 255
#endif

#ifndef PAN_TILT_SERVOS
#define PAN_TILT_SERVOS 1
#endif

#ifndef TOTAL_FUNCTIONS
#define TOTAL_FUNCTIONS 1
#endif

#ifndef POSITION1 //This is position 1
#define POSITION1 150
#endif

#ifndef POSITION2 //This is position 2
#define POSITION2 196
#endif

#ifndef POSITION3 //This is position 3
#define POSITION3 242
#endif

#ifndef POSITION4 //This is position 4
#define POSITION4 288
#endif

#ifndef POSITION5 //This is position 5
#define POSITION5 333
#endif

#ifndef POSITION6 //This is position 6
#define POSITION6 379
#endif

#ifndef POSITION7 //This is position 7
#define POSITION7 425
#endif

#ifndef POSITION8 //This is position 8
#define POSITION8 470
#endif

#ifndef POSITION9 //This is position 9
#define POSITION9 545
#endif

#ifndef TOTAL_POSITIONS
#define TOTAL_POSITIONS 10
#endif

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


//Connect pin 6 on Arduino to orange wire on servo, red wire on servo to +5V and brown wire on servo to GND.

/*
From Atmega2560 datasheet

freq OCnx = F_CPU / (PRESCALAR * (1 + TOP))
where TOP = ICRn in our case for mode 14 fast PWM

e.g. to create 50 Hz pulse as required by SG90 servo motor
50 = 16 MHz / ( 64 * (1 + ICRn))
(16 MHz / ( 50 * 64 )) - 1 = ICRn
ICRn = 4999
*/

//===================Servo Motor ============================================
//minus one to discard UNKNOWN position
//static unsigned long positions[2] = {POSITION1, POSITION2};
static unsigned long positions[9] = {POSITION1, POSITION2, POSITION3, POSITION4, POSITION5, POSITION6, POSITION7, POSITION8, POSITION9};
static const char* position_pan[9] = {"POS1", "POS2", "POS3", "POS4", "POS5", "POS6", "POS7", "POS8", "POS9"};
static const char* position_tilt[9] = {"POS1", "POS2", "POS3", "POS4", "POS5", "POS6", "POS7", "POS8", "POS9"};
static unsigned long division(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor/2)) / divisor;
}


uint8_t getIndexPosition(unsigned long pos) {
   uint8_t i;
   for(i=0; i<sizeof(positions)/sizeof(unsigned long); i++){
       if(pos == positions[i]){
          break;
       }
   } 
   return i;
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

void blink_led_slow()
{
  /* set pin 7 high to turn led on */
  PORTB |= _BV(PORTB7);
  _delay_ms(BLINK_DELAY_MS_SLOW);
 
  /* set pin 7 low to turn led off */
  PORTB &= ~_BV(PORTB7);
  _delay_ms(BLINK_DELAY_MS_SLOW);
}

void blink_led_fast()
{
  /* set pin 7 high to turn led on */
  PORTB |= _BV(PORTB7);
  _delay_ms(BLINK_DELAY_MS_FAST);
 
  /* set pin 7 low to turn led off */
  PORTB &= ~_BV(PORTB7);
  _delay_ms(BLINK_DELAY_MS_FAST);
}



//Read incoming data in the interrupt
volatile int i=0;
char buffer[255]= {};
char buffer_send[255]= {};
//volatile int16_t position = UNKNOWN_POSITION;
volatile int16_t position_servo1 = POSITION1;
volatile int16_t position_servo2 = POSITION1;
volatile int16_t LAST_POSITION_SERVO1 = POSITION1;
volatile int16_t LAST_POSITION_SERVO2 = POSITION1;
volatile int8_t function = UNKNOWN_FUNCTION;
volatile uint8_t recv_flag=0;
ISR(USART1_RX_vect)
{
    buffer[i]=UDR1;         //Read USART data register
    if(buffer[i++]=='\r')   //check for carriage return terminator and increment buffer index
    {
        //Make a copy
        memset(buffer_send, '\0', sizeof(buffer_send));
        //strncpy(buffer_send, buffer, sizeof(buffer-1));

        //Now parse the string
        //j represent fields separated by comma delimiter.
        uint8_t j=0;
        const char delimiter[2] = ",";
        char *token=NULL;
        /* get the first token */
        token = strtok(buffer, delimiter);
        if((j==0) && (token != NULL)) {
            if(strncmp(token,"PT",2)==0) {
                //blink_led_slow();
                function = PAN_TILT_SERVOS;
            }
            if(strncmp(token,"PT_POS",6)==0) {
                strncat(buffer_send,position_pan[getIndexPosition(LAST_POSITION_SERVO1)],4);
                strncat(buffer_send,",",1);
                strncat(buffer_send,position_tilt[getIndexPosition(LAST_POSITION_SERVO2)],4);
                strncat(buffer_send,"\r\n",2);
            }
        }
        /* walk through other tokens */
        while( token != NULL ) {
            j++;
            token = strtok(NULL, delimiter);
            if((j==1) && (token != NULL)) {
                /*set the position*/
                if((strncmp(token,"POS1",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION1;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS2",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION2;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS3",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION3;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS4",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION4;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS5",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION5;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS6",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION6;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS7",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION7;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS8",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION8;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS9",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = POSITION9;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"NONE",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo1 = LAST_POSITION_SERVO1;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    //strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                }
                /*
                } else if (strncmp(token,"SOME",4)==0) {
                   function = UNKNOWN_FUNCTION;
                }
                */
            }
            if((j==2) && (token != NULL)) {
                strncat(buffer_send,",",1);
                /*set the position*/
                if((strncmp(token,"POS1",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION1;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS2",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION2;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS3",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION3;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS4",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION4;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS5",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION5;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS6",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION6;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS7",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION7;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS8",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION8;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"POS9",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = POSITION9;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                } else if ((strncmp(token,"NONE",4)==0)&&(function==PAN_TILT_SERVOS)) {
                    //blink_led_fast();
                    position_servo2 = LAST_POSITION_SERVO2;
                    //copy to char buffer to be echoed back to the receiver
                    strncat(buffer_send,token,strlen(token));
                    strncat(buffer_send,"\r\n",2);
                    //strncat(buffer_send,",",1);
                }
                /*
                } else if (strncmp(token,"SOME",4)==0) {
                   function = UNKNOWN_FUNCTION;
                }
                */
            }


        }

        // if terminator detected

        //strncpy(buffer_send, buffer, sizeof(buffer_send));
        //recv_flag=1;        //Set String received flag
        buffer[i-1]=0x00;   //Set string terminator to 0x00
        i=0;                //Reset buffer index
        recv_flag=1;        //Set String received flag
    }
}



int main(void)
{
    //SG90 servo needs 50 Hz or 20 milliseconds pulse
    /*

	Weight: 9 g 
	Dimension: 22.2 x 11.8 x 31 mm approx. 
	Stall torque: 1.8 kgf-cm 
	Operating speed: 0.1 s/60 degree 
	Operating voltage: 4.8 V (~5V) 
	Dead band width: 10 Î¼s 
	Temperature range: 0 ÂºC â€“ 55 ÂºC 

        Leads: PWM (Orange), Vcc (Red), Ground (Brown)
        PWM Period: 20 milliseconds
        duty cycle varies between 1 and 2 ms
        Position -90 (1 milliseconds/20 milliseconds -> 5% duty cycle) - left position
        Position 0 (1.5 milliseconds/20 milliseconds -> 7.5% duty cycle)  - middle position
        Position +90 (2 milliseconds/20 milliseconds -> 10% duty cycle) - right position

    */

    unsigned long timer_frequency;
    unsigned long desired_frequency = 50; // 50 Hz
    const unsigned long prescalar = 64;
    //unsigned long duty_cycle_start;
    //unsigned long duty_cycle;
    //unsigned long duty_cycle_end;

    // Initial PORT 
  
    //Set PORTH to OUTPUT
    DDRH |= _BV(DDH3);
    //Set the output port H pin 3 to 0 (LOW) state
    PORTH &= ~_BV(PORTH3);

    //Set PORTH to OUTPUT
    DDRH |= _BV(DDH4);
    //Set the output port H pin 3 to 0 (LOW) state
    PORTH &= ~_BV(PORTH4);


    /* set pin 7 of PORTB for onboard LED blinking output*/
    DDRB |= _BV(DDB7);
 
    //Clear Interrupts
    cli(); 

    // =========== Servo Motor initialization begin ================

    timer_frequency = division(F_CPU, prescalar);
    ICR4 = division(timer_frequency, desired_frequency) - 1;

    // Initial TIMER4 Fast PWM
    // Fast PWM Frequency = fclk / (N * TOP), Where N is the Prescaler
    TCCR4A |= 1<<WGM41 | 0<<WGM40; // Fast PWM - Mode 14 with 16 Bit timer
    TCCR4B |= 1<<WGM43 | 1<<WGM42; // Fast PWM - Mode 14 with 16 Bit timer
    //Clear OC4A on compare match, set OC4A at BOTTOM (non-inverting mode)
    TCCR4A |= 1<<COM4A1 | 0<<COM4A0;
    TCCR4A |= 1<<COM4B1 | 0<<COM4B0;
    // Used 64 Prescaler
    TCCR4B |= 0<<CS12 | 1<<CS11 | 1<<CS10;  //Divide by 64 (We are using 16 MHz external Crystal)  
    // =========== Servo Motor initialization end ================


    
    // =========== usart initialization begin ================
    USART_Init();   // Initialise USART
    // =========== usart initialization end ================

    //Set Interrupts
    sei();

    OCR4A = position_servo1;
    OCR4B = position_servo2;
 
    unsigned int i = 0;
    while(1)
    {
        if (recv_flag)
        {
           //Echo the string back
           USART_putstring(buffer_send);

           //OCR4A = positions[i];
           OCR4A = position_servo1;
           LAST_POSITION_SERVO1 = position_servo1;
           OCR4B = position_servo2;
           LAST_POSITION_SERVO2 = position_servo2;
	   //_delay_ms(500);

           recv_flag=0;                // Reset String received flag
        }

        /*
        // We vary the duty cycle between 3% and 11% @5V -> To move from 0 degrees to 180 degrees
        for(i = 0; i < sizeof(positions)/sizeof(unsigned long); i++){
           OCR4A = positions[i];
	   _delay_ms(500);
        }
        //simple logic to reset the count
        if (i >= sizeof(positions)/sizeof(unsigned long)) {
            i = 0;
        }
        */
    }
    return 0;
}
