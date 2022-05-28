#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#include <avr/io.h>

#define AVR_JTAG_IR_LENGTH 4

#define TDI DDD6
#define TDO DDB7
#define TMS DDB6
#define TCK DDB5

typedef enum state
{
    LOW = 0,
    HIGH = 1
} state_t;

void enableJtag();

int countDevices();

void resetJtagFsm();

void toggleClock();

uint8_t getTDO();

void writeInstruction(OPCODES);