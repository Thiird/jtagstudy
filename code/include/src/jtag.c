#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "../header/jtag.h"

void toggleClock()
{
    PORTB |= (1 << PORTB5);
    _delay_us(1); // 1us, as if 1MHz
    PORTB &= ~(1 << PORTB5);
}

uint8_t isJtagEnabled()
{
    // TODO
    //  read JTAGEN fuse

    return 1;
}

uint8_t getTDO()
{
    return (PINB & (1 << TDO)) >> 7;
}

void setTDI(state_t state)
{
    if (state)
        PORTD |= (1 << TDI);
    else
        PORTD &= ~(1 << TDI);
}

void setTMS(state_t state)
{
    if (state)
        PORTB |= (1 << TMS);
    else
        PORTB &= ~(1 << TMS);
}

uint8_t countTapChainLenght()
{
    uint8_t nOfDevices = 0;

    // go to 'Run Test/Idle' state
    setTMS(LOW);
    toggleClock();

    // go to 'Select DR-Scan' state
    setTMS(HIGH);
    toggleClock();

    // go to 'Select IR-Scan' state
    toggleClock();

    // go to 'Capture IR' state
    setTMS(LOW);
    toggleClock();

    // go to 'Shift IR' state
    toggleClock();

    // shift in BYPASS instruction (all 1's)
    setTDI(HIGH);
    for (uint8_t i = 0; i < AVR_JTAG_IR_LENGTH - 1; i++)
        toggleClock();

    // go to 'Exit-1 IR' state and shift in
    // last istruction bit (still 1)
    toggleClock();

    // go to 'Update IR' state
    // after this istruction is operational
    toggleClock();

    // now we are ready to shift in the test bits into TDI
    // and see with which offset they comeout of TDO
    // the offset will determine the lenght of the TAP chain
    // because in the BYPASS mode, each TAP controller will
    // simply delay TDO in respect to TDI by exactly one clock

    // go to 'SeleRun Test/Idle' state
    setTMS(LOW);
    toggleClock();

    uint8_t testDataIn = 0b10111101;
    uint8_t testDataOut = 0;

    for (uint8_t i = 0; i < sizeof(testDataIn) * 8; i++)
    {
        // send i-th bit of testDataIn to TDI
        setTDI(testDataIn & (1 << i));
        toggleClock();

        // save current value of TDO
        testDataOut |= getTDO() << i;
    }

    return nOfDevices;
}

void resetJtagFsm()
{
    // clock TMS HIGH for 5 cycles to reset FSM
    PORTB |= (1 << PORTB6);
    for (int i = 0; i < 5; i++)
        toggleClock();
    PORTB &= ~(1 << PORTB6);
}

void writeInstruction();
