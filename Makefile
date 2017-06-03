file=led1
MMCU=atmega2560
all:
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=$(MMCU) -c -o $(file).o $(file).c
	avr-gcc -mmcu=$(MMCU) $(file).o -o $(file)
	#avr-objcopy -O ihex -R .eeprom $(file) $(file).hex
	avr-objcopy -O ihex -R .flash $(file) $(file).hex
burn:
	sudo avrdude -c wiring -p m2560 -P /dev/ttyACM0 -b 115200 -V -U flash:w:$(file).hex -C avrdude.conf -D
