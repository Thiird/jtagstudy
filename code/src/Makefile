FILE?=main
PROGRAMMER?=usbasp
PORT?=/dev/ttyACM0
BAUD?=115200

LFUSEX=$(shell avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U lfuse:r:-:h -q -q)
HFUSEX=$(shell avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U hfuse:r:-:h -q -q)
EFUSEX=$(shell avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U efuse:r:-:h -q -q)

LFUSEB=$(shell avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U lfuse:r:-:b -q -q)
HFUSEB=$(shell avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U hfuse:r:-:b -q -q)
EFUSEB=$(shell avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U efuse:r:-:b -q -q)

default: build flash

build:
	avr-gcc -mmcu=atmega32u4 main.c ../include/src/usart.c ../include/src/jtag.c -o bin/main

flash:
	avr-objcopy -O ihex -R .eeprom bin/main bin/main.hex
	avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U flash:w:bin/main.hex

clean:
	rm -rf bin

description:
	avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -v

writefuses:
	avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -U hfuse:w:0x98:m

fuses:
	@echo
	@echo "Note: 0 means programmed, 1 means not programmed"
	@echo
	@echo "Low Fuse Byte:"
	@echo -n -e ${LFUSEX}" - "${LFUSEB}"\n"
	@echo
	@echo "High Fuse Byte:"
	@echo -n -e ${HFUSEX}" - "${HFUSEB}"\n"
	@echo
	@echo "Extended Fuse Byte:"
	@echo -n -e ${EFUSEX}" - "${EFUSEB}"\n"
	@echo

jtagenable:
	avrdude -c ${PROGRAMMER} -p m32u4	 -P ${PORT} -b ${BAUD} -U hfuse:w:0x98:m

jtagdisable:
	avrdude -c ${PROGRAMMER} -p m32u4 -P ${PORT} -b ${BAUD} -U hfuse:w:0xD8:m