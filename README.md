# Elegoo_Mega_2560
AVRGCC example source files for Elegoo Atmega2560 board.

*How to Use*

1. In the **Makefile** set the '**file**' variable to C source as one wishes (without file extension). e.g. **file=led1**
2. Then issue command '**make**'
3. Then to load the hex file issue the command '**make burn**' after connecting USB cable to Elegoo Atmega 2560.
   **Note:** This will require the user to enter password.
   
   **Note:** avrdude.conf file was adapted from Arduino Linux tgz package.
