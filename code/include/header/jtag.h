#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#include <avr/io.h>

#define AVR_JTAG_IR_LENGTH 4
#define MAX_TAP_CHAIN_LENGTH 64
#define IDCODE_LENGTH 32

// physical pins on 32u4 used for bit-banging
// {D6, B7, B6,}

#define LOW (uint8_t)0
#define HIGH (uint8_t)1

uint8_t isJtagEnabled();

void setJtagInterface();

uint8_t getTapChainLenght();

void resetJtagFsm();

void initHwPins();

void setRegister(volatile uint8_t *reg, uint8_t number, uint8_t value);

void getDeviceIds();

void findJtagInterface();

void toggleClock();

uint8_t getTDO();