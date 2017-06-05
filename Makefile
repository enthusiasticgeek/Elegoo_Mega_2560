file=blink_led1
MMCU=atmega2560
PROTOCOL=wiring
ATMEGA=m2560
DEVICE=/dev/ttyACM0
BAUD=115200
all:
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=$(MMCU) -c -o $(file).o $(file).c
	avr-gcc -mmcu=$(MMCU) $(file).o -o $(file)
	#avr-objcopy -O ihex -R .eeprom $(file) $(file).hex
	avr-objcopy -O ihex -R .flash $(file) $(file).hex
burn:
	sudo avrdude -c $(PROTOCOL) -p $(ATMEGA) -P $(DEVICE) -b $(BAUD) -V -U flash:w:$(file).hex -C avrdude.conf -D
