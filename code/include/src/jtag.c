#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "../header/jtag.h"
#include "../header/usart.h"

void toggleClock()
{
    PORTB |= (1 << TCK);
    _delay_ms(10);
    PORTB &= ~(1 << TCK);
    _delay_ms(10);
}

uint8_t isJtagEnabled()
{
    // TODO
    // read JTAGEN fuse

    return (uint8_t)1;
}

uint8_t getTDO()
{
    return ((PINB & (1 << TDO)) >> TDO);
}

void setTDI(uint8_t state)
{
    if (state)
        PORTD |= (1 << TDI);
    else
        PORTD &= ~(1 << TDI);
    _delay_ms(2);
}

void setTMS(uint8_t state)
{
    if (state)
        PORTB |= (1 << TMS);
    else
        PORTB &= ~(1 << TMS);
    _delay_ms(2);
}

void initJtagInterface()
{
    DDRD |= (1 << TDI);  // output
    DDRB &= ~(1 << TDO); // input
    DDRB |= (1 << TMS);  // output
    DDRB |= (1 << TCK);  // output

    // ensure TCK starts LOW
    PORTB &= ~(1 << TCK);
}

void moveFSM(uint8_t bit)
{
    setTMS(bit);
    toggleClock();
}

uint8_t countTapChainLenght()
{
    // assuming jtag fsm is in 'Test-Logic-Reset' state

    moveFSM(LOW);  // moving to 'Run-Test/Idle' state
    moveFSM(HIGH); // moving to 'Select DR-Scan' state
    moveFSM(HIGH); // moving to 'Select IR-Scan' state
    moveFSM(LOW);  // moving to 'Capture IR' state
    moveFSM(LOW);  // moving to 'Shift IR' state

    // shift in BYPASS instruction (all 1's)
    setTDI(HIGH);
    for (uint8_t i = 0; i < AVR_JTAG_IR_LENGTH - 1; i++)
        toggleClock();
    // last bit of instruction must have TMS high to exit 'shift-IR' state
    moveFSM(HIGH);
    setTDI(LOW);

    // now we are in 'Exit1-IR' state, let's go to 'Shift-DR' state

    // this tms sequence corrisponds to:
    moveFSM(HIGH); // go to 'Update IR' state, after this, loaded instruction is operational
    // now we are ready to shift in the test bits into TDI
    // and see with which offset they come out of TDO
    // the offset will determine the lenght of the TAP chain
    // because in the BYPASS mode, each TAP controller will
    // simply delay TDO in respect to TDI by exactly one clock
    moveFSM(HIGH); // go to 'Select DR-Scan' state
    moveFSM(LOW);  // go to 'Capture-DR' state
    moveFSM(LOW);  // go to 'Shift-DR' state

    // send plenty of 0's to flush the BYPASS registers
    setTDI(LOW);
    for (uint8_t i = 0; i < 64; i++)
        toggleClock();

    // set TDI HIGH and count how many
    // clocks it takes to show up on TDO
    setTDI(HIGH);
    int cont = 0;
    for (uint8_t i = 0; i < 64; i++)
    {
        if (getTDO())
            return i;

        toggleClock();
    }
    setTDI(LOW); // reset line

    return LOW;
}

void resetJtagFsm()
{
    usartSend("Resetting JTAG FSM... ");
    // clock TMS HIGH for 5 cycles to reset FSM
    // into 'Test-Logic-Reset' state
    setTDI(LOW); // for safety set TDI to 0
    setTMS(HIGH);
    for (int i = 0; i < 5; i++)
        toggleClock();
    setTMS(LOW);
    usartSend("Done\n\r");
}

void writeInstruction();
