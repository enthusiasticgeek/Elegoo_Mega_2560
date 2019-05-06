//Copyright (c) 2019 Pratik M Tambe <enthusiasticgeek@gmail.com>
//Code modified from the following URL
//https://www.electronicwings.com/avr-atmega/interfacing-lcd-16x2-in-4-bit-mode-with-atmega-16-32-

#include <avr/io.h>						/* Include AVR std. library file */
#include <util/delay.h>					/* Include inbuilt defined Delay header file */
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h> // not required for primary operations.
#include <stdlib.h> 
// http://www.electroschematics.com/10053/avr-adc/
//Benn Thomsen has a good article on this. This is the basis for the following program. See below.
// Reference: https://bennthomsen.wordpress.com/arduino/peripherals/analogue-input/

//Atmega 2560 has 10-bit successive approximation type ADC @ 15kS/s sampling rate.
//ADC modes: Single Conversion, Triggered Conversion, Free Running (We use the last one here)

/*
From Atmega2560 datasheet
    ADC Prescaler: 7-bit Selected using ADPS0, ADPS1, ADPS2 (2, 4, 16, 32, 64 and 128) - ADC frequency between 50KHz to 200KHz (> 10 bit resolution)
    ADC Channels: The ADC in Atmega2560 PDIP package has 16 channels, allows you to take samples from 16 different pins
    ADC Registers: Register provides the communication link between CPU and the ADC. You can configure the ADC according to your need using these registers. The ADC has 3 registers only:
        ADC Multiplexer Selection Register – ADMUX: For selecting the reference voltage and the input channel
        ADC Control and Status Register A – ADCSRA: It has the status of ADC and is also used to control it
        The ADC Data Register – ADCL and ADCH: Final result of the conversion is stored here
    AVCC: This pin supplies power to ADC. AVCC must not differ more than ± 0.3V from Vcc
    AREF: Another pin which can optionally be used as an external voltage reference pin.
    Voltage Resolution:This is the smallest voltage increment which can be measured. For 10 bit ADC, there can be 1024 different voltages (for an 8 bit ADC, there can be 256 different voltages)
*/

//Connect 10K Ohms POTENTIOMETER's center to A7 pin on ADC port of Arduino. One side of POT to to GND. Remaining side of POT to +5V. Connect ARef to +5V and Capacitor 0.1 microFarad between GND and +5V.





//Target: = Atmega2560 @ 16 MHz (tested)
//---------------DEFINITIONS--------------------------

//LCD_VSS - pin 1 of LCD -> GND
//LCD_VDD - pin 2 of LCD -> 5V
//LCD_V0 - pin 3 of LCD -> connect to 2.2K -> GND
//LCD A - pin 15 -> 5V
//LCD K - pin 16 -> GND
#define LCD_RS_SIGNAL	      0 	      //PC0 pin37 of uC to LCD RS - pin 4 of LCD
#define LCD_RW_SIGNAL	      1 	      //PC1 pin36 of uC (don't connect) to LCD R/W - currently grounded
#define LCD_ENABLE_SIGNAL     2  	      //PC1 pin36 of uC to LCD E - pin 6 of LCD
#define LCD_C4_SIGNAL	      4	              //PC4 pin33 of uC to LCD D3 - pin 11 of LCD
#define LCD_C5_SIGNAL	      5	              //PC5 pin32 of uC  to LCD D4 - pin 12 of LCD
#define LCD_C6_SIGNAL	      6	              //PC6 pin31 of uC to LCD D5 - pin 13 of LCD
#define LCD_C7_SIGNAL	      7	              //PC7 pin30 of uC to LCD D6 - pin 14 of LCD



#define LCD_Dir DDRC					/* Define LCD data port direction */
#define LCD_Port PORTC					/* Define LCD data port */
#define RS PC0							/* Define Register Select (data reg./command reg.) signal pin */
#define EN PC1 							/* Define Enable signal pin */
 

void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);				/* RS=0, command reg. */
	LCD_Port |= (1<<EN);				/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);				/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_string(char *s, int LCD_line, int LCD_cursor_position)
{
    if (LCD_line == 1)
        LCD_Command(0x80 + LCD_cursor_position);
    else
        LCD_Command(0xc0 + LCD_cursor_position);
  while (*s)       
  LCD_Char(*s++); // ++ is equivalent to increment
}

void LCD_cursor_blink(void)	//displays LCD blinking cursor
{
	LCD_Command(0x0F);
}

void LCD_string_Float(float LCD_number, int LCD_line, int LCD_cursor_position)
{
   char buffer_LCD[13];

   dtostre(LCD_number, buffer_LCD, 5, 4); 
   LCD_string(buffer_LCD, LCD_line, LCD_cursor_position);
}


void LCD_string_Int(int LCD_number, int LCD_line, int LCD_cursor_position)
{
    char buffer_LCD[10];
    
    itoa(LCD_number, buffer_LCD, 10);
    LCD_string(buffer_LCD, LCD_line, LCD_cursor_position);
}



void LCD_Init (void)					/* LCD Initialize function */
{
	LCD_Dir = 0xFF;						/* Make LCD command port direction as o/p */
	_delay_ms(20);						/* LCD Power ON delay always >15ms */
	
	LCD_Command(0x33);
	LCD_Command(0x32);		    		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              	/* Use 2 line and initialize 5*7 matrix in (4-bit mode)*/
	LCD_Command(0x0c);              	/* Display on cursor off*/
	LCD_Command(0x06);              	/* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              	/* Clear display screen*/
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}


void LCD_String (char *str)				/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)				/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	LCD_String(str);					/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);					/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}

void LCD_Home(void)			//LCD cursor home
{
	LCD_Command(1<<1);
}
 
unsigned int adc_value;
int main()
{

    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0);
    //Clear Interrupts
    cli(); 
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC -> prescaler to 128 - 125KHz sample rate @ 16MHz
    ADCSRB |= (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0); // Set ADC -> Free Running Mode
    ADMUX |= (0 << REFS1)| (1 << REFS0); // Set ADC reference to AVCC
    //ADMUX |= (0 << REFS1)| (0 << REFS0); //AREF, Internal VREF  turned off
    //ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit
    ADMUX |= (0 << ADLAR); // Default - right adjust ADC result to allow easy 8 bit
    ADCSRA |= (1 << ADEN); // Enable ADC
    ADCSRA |= (1 << ADIE); // Enable ADC Interrupt
    ADMUX |= (1 << MUX2)| (1 << MUX1) |(1 << MUX0); // using ADC7 pin 
    //Set Interrupts
    sei();

	LCD_Init();							/* Initialization of LCD*/
        LCD_cursor_blink();
	LCD_String("EnthusiasticGeek");		/* Write string on 1st line of LCD*/
	LCD_Command(0xc0);					/* Go to 2nd line*/
	LCD_String("Happy Coding!");			/* Write string on 2nd line*/
        //LCD_Clear();
        //LCD_string_Float(0.034,1,1);
        //LCD_Clear();
        //LCD_string_Int(3000,1,1);
	while(1){

             ADCSRA |= (1<<ADSC); // Start conversion
             //Instead of logic below for waiting for conversion we wait for ADC conversion complete interrupt
             //while (ADCSRA & (1<<ADSC)); // wait for conversion to complete
   LCD_Clear();
   LCD_string_Int(adc_value,1,1);
   _delay_ms(500);
   _delay_ms(500);

        }
}
 

ISR(ADC_vect){
   //ADCW stores the value
   adc_value = ADCW;
/*
   // max ADCW value is 2^10 = 1024 hence half of it. 
   if (adc_value < 512)
   {
   } else {

   }
*/

}
