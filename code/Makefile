${FILE}?=main

default: build flash

build:
	avr-gcc -mmcu=atmega32u4 ${FILE}.c -o bin/${FILE}

flash:
	avr-objcopy -O ihex -R .eeprom bin/${FILE} bin/${FILE}.hex
	avrdude -F -V -c avr109 -p m32u4 -P /dev/ttyACM0 -b 115200 -U flash:w:bin/${FILE}.hex

clean:
	rm -f /bin