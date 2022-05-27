FILE?=main
PROGRAMMER?=usbasp
PORT?=/dev/ttyACM0
BAUD?=115200

default: build flash

build:
	avr-gcc -mmcu=atmega32u4 main.c ../include/src/usart.c -o bin/main

flash:
	avr-objcopy -O ihex -R .eeprom bin/main bin/main.hex
	avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U flash:w:bin/main.hex

clean:
	rm -rf bin
