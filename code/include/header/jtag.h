#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#include <avr/io.h>

#define AVR_JTAG_IR_LENGTH 4

#define TDI PD6
#define TDO PB7
#define TMS PB6
#define TCK PB5

#define LOW (uint8_t)0
#define HIGH (uint8_t)1

uint8_t isJtagEnabled();

void initJtagInterface();

uint8_t countTapChainLenght();

void resetJtagFsm();

void toggleClock();

uint8_t getTDO();

void writeInstruction();