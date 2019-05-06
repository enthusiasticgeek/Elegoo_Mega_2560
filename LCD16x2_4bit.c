//Copyright (c) 2019 Pratik M Tambe <enthusiasticgeek@gmail.com>
//Code modified from the following URL
//https://www.electronicwings.com/avr-atmega/interfacing-lcd-16x2-in-4-bit-mode-with-atmega-16-32-

#include <avr/io.h>						/* Include AVR std. library file */
#include <util/delay.h>					/* Include inbuilt defined Delay header file */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h> // not required for primary operations.
#include <stdlib.h> 

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
 
int main()
{

	LCD_Init();							/* Initialization of LCD*/
        LCD_cursor_blink();
	LCD_String("EnthusiasticGeek");		/* Write string on 1st line of LCD*/
	LCD_Command(0xc0);					/* Go to 2nd line*/
	LCD_String("Happy Coding!");			/* Write string on 2nd line*/
        //LCD_Clear();
        //LCD_string_Float(0.034,1,1);
        //LCD_Clear();
        //LCD_string_Int(3000,1,1);
	while(1);
}
 
