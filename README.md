# Elegoo_Mega_2560
AVRGCC example source files for Elegoo Atmega2560 board.

**Pre-requisites: (Tested on Ubuntu 16.04 LTS)**

1. sudo apt-get install gcc-avr avr-libc avrdude 
2. Some Editor (vim, emacs, eclipse, gedit, notepad++, etc) 

**How to Use**

1. Issue the following command while setting the '**file**' variable to C source as one wishes (without file extension) for building source code. e.g. for *blink_led.c* source file the command is **make file=blink_led**
2. Then to load the hex file (e.g. blink_led) issue the command **make burn file=blink_led** after connecting USB cable to Elegoo Atmega 2560.
   **Note:** This will require the user to enter password.
   
   **Note:** avrdude.conf file was adapted from Arduino Linux tgz package.
      
**References:**
   
1. Freedom Embedded Blog by Balau https://balau82.wordpress.com/arduino-in-c/
2. Hackaday AVR programming http://hackaday.com/2010/10/23/avr-programming-introduction/
3. Elegoo https://www.elegoo.com/download/
