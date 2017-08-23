# Elegoo_Mega_2560
AVRGCC example source files for Elegoo Atmega2560 board.
![alt text](https://github.com/enthusiasticgeek/Elegoo_Mega_2560/blob/master/arduino_2560.jpg "ARDUINO")
![alt text](https://www.elegoo.com/wp-content/uploads/2017/01/2-14.jpg "Elegoo")

**Pre-requisites (Tested on Ubuntu 16.04 LTS)**

1. sudo apt-get install gcc-avr avr-libc avrdude 
2. Some Editor (vim, emacs, eclipse, gedit, notepad++, etc) 

**Datasheet**

http://www.atmel.com/Images/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf

**Schematics**

http://download.arduino.org/products/MEGA2560/Arduino_MEGA_2560-Rev3d-SCH.pdf

https://www.arduino.cc/en/uploads/Main/arduino-mega2560-schematic.pdf

**Goal**

To teach the reader how to program Arduino based on AVR ATMEGA 2560 directly using avrgcc. Hence, I have tried to keep the code as simple as possible.

**How to Use**

1. Issue the following command while setting the '**file**' variable to C source as one wishes (without file extension) for building source code. (e.g. for *blink_led.c*) source file issue the command **make file=blink_led**
2. Then to load the hex file (e.g. for *blink_led.hex*) hex file issue the command **make burn file=blink_led** after connecting USB cable to Elegoo Atmega 2560.
   **Note:** This will require the user to enter password.
   
   **Note:** avrdude.conf file was adapted from Arduino Linux tgz package.
      
**References**
   
1. Freedom Embedded Blog by Balau https://balau82.wordpress.com/arduino-in-c/
2. Hackaday AVR programming by Mike Szczys http://hackaday.com/2010/10/23/avr-programming-introduction/
3. Elegoo https://www.elegoo.com/download/
4. AVR atmega PWM http://www.ermicro.com/blog/?p=1971
