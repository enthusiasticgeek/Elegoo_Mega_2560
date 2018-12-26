#//(c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
file=blink_led
MMCU=atmega2560
PROTOCOL=wiring
ATMEGA=m2560
DEVICE=/dev/ttyACM0
BAUD=115200
all:
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=$(MMCU) -c -o $(file).o $(file).c -Wl,-u,vfprintf -lprintf_flt -lm
	avr-gcc -mmcu=$(MMCU) $(file).o -o $(file)
	#EEPROM
	#avr-objcopy -O ihex -R .eeprom $(file) $(file).hex
	#FLASH
	avr-objcopy -O ihex -R .flash $(file) $(file).hex
burn:
	sudo avrdude -c $(PROTOCOL) -p $(ATMEGA) -P $(DEVICE) -b $(BAUD) -V -U flash:w:$(file).hex -C avrdude.conf -D
