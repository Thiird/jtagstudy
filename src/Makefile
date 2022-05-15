default: build flash

build:
	avr-gcc -mmcu=atmega32u4 main.c -o main

flash:
	avr-objcopy -O ihex -R .eeprom main main.hex
	stty -F /dev/ttyACM0 speed 1200
	sleep 1
	avrdude -F -V -c avr109 -p m32u4 -P /dev/ttyACM0 -b 115200 -U flash:w:main.hex

clean:
	rm -f *.hex *.o