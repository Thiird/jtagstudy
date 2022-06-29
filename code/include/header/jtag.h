#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#include <avr/io.h>

#define AVR_JTAG_IR_LENGTH 4
#define MAX_TAP_CHAIN_LENGTH 64
#define IDCODE_LENGTH 32

#define TDI DDD6
#define TDO PINB7
#define TMS DDB6
#define TCK DDB5

#define LOW (uint8_t)0
#define HIGH (uint8_t)1

uint8_t isJtagEnabled();

void initJtagInterface();

uint8_t getTapChainLenght();

void resetJtagFsm();

void getDeviceIds();

void toggleClock();

uint8_t getTDO();