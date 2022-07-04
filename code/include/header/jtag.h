#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#include <avr/io.h>

#define AVR_JTAG_IR_LENGTH 4
#define MAX_TAP_CHAIN_LENGTH 64
#define IDCODE_LENGTH 32
#define AVAILABLE_PINS 4 // pins on which we can search a jtag interface

#define TDI_ID 0
#define TDO_ID 1
#define TMS_ID 2
#define TCK_ID 3

#define LOW (uint8_t)0
#define HIGH (uint8_t)1

typedef struct
{
    volatile uint8_t *dreg; // direction register
    volatile uint8_t *wreg; // write register
    volatile uint8_t *rreg; // read register
    uint8_t number;
} pin;

void setTDI(uint8_t state);

void setTMS(uint8_t state);

uint8_t getTDO();

void toggleClock();

void setRegister(volatile uint8_t *reg, uint8_t number, uint8_t value);

uint8_t isJtagEnabled();

void resetJtagFsm();

void initJtag();

void initHwPins();

void setJtagInterface();

void moveFSM(uint8_t bit);

uint8_t getTapChainLenght();

void getDeviceIds();

uint8_t findJtagInterface();
