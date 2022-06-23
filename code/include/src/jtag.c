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

    return 1;
}

uint8_t getTDO()
{
    return (PINB & (1 << TDO)) ? (uint8_t)1 : (uint8_t)0;
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
    PORTC &= ~(1 << TCK);
}

uint8_t countTapChainLenght()
{
    // assuming jtag fsm is in 'Test-Logic-Reset' state

    uint16_t tmsSequence = 0b01100;
    // this tms sequence corrisponds to:
    // moving to 'Run-Test/Idle' state
    // moving to 'Select DR-Scan' state
    // moving to 'Select IR-Scan' state
    // moving to 'Capture IR' state
    // moving to 'Shift IR' state

    for (uint8_t i = 4; i > 0; i--)
    {
        setTMS(tmsSequence & ((uint16_t)1 << i) ? (uint8_t)1 : (uint8_t)0);
        toggleClock();
    }

    // shift in BYPASS instruction (all 1's)
    setTDI(HIGH);
    for (uint8_t i = 0; i < AVR_JTAG_IR_LENGTH - 1; i++)
        toggleClock();
    // last bit of instruction must have TMS high to exit 'shift-IR' state
    setTMS((uint8_t)1);
    toggleClock();

    // now we are in 'Exit1-IR' state, let's go to 'Shift-DR' state

    tmsSequence = 0b1100;
    // this tms sequence corrisponds to:
    // go to 'Update IR' state, after this, loaded instruction is operational
    // now we are ready to shift in the test bits into TDI
    // and see with which offset they come out of TDO
    // the offset will determine the lenght of the TAP chain
    // because in the BYPASS mode, each TAP controller will
    // simply delay TDO in respect to TDI by exactly one clock
    // go to 'Run Test/Idle' state
    // go to 'Select DR-Scan' state
    // go to 'Capture-DR' state

    for (uint8_t i = 3; i > 0; i--)
    {
        setTMS(tmsSequence & ((uint16_t)1 << i));
        toggleClock();
    }

    // send plenty of 0's to flush the registers
    setTDI(HIGH);
    for (uint8_t i = 5; i > 0; i--)
        toggleClock();

    //  send test sequence
    uint8_t test = 0b01101100;
    uint8_t size = sizeof(test) << 3; // multiply by 8
    uint8_t result = 0;
    uint8_t bit = 0;

    setTDI(HIGH);
    for (uint8_t i = 1; i < 5; i++)
    {
        if (getTDO())
            return i;
    }

    /*usartSend("Sending test sequence:\n\r");
    for (uint8_t i = 0; i < size; i++)
    {
        bit = (test & ((uint8_t)1 << i)) ? (uint8_t)1 : (uint8_t)0;
        usartSend("Sent: %d\n\r", bit);
        setTDI(bit);
        toggleClock();

        // save current value of TDO
        usartSend("Received: %d\n\r", getTDO());
        result |= getTDO() << i;
    }*/
    /*
        usartSend("Received:\n\r");
        for (uint8_t i = 0; i < size; i++)
        {
            usartSend("%d", (result & (1 << (size - i - 1))) >> (size - i - 1));
        }
        usartSend("\n\r");*/

    // find shift
    /*for (uint8_t i = 1; i < size; i++)
    {
        if ((test << i) & result == test << i)
            return i;
    }*/

    return LOW;
}

void resetJtagFsm()
{
    usartSend("Resetting JTAG FSM... ");
    // clock TMS HIGH for 5 cycles to reset FSM
    // into 'Test-Logic-Reset' state
    setTMS((uint8_t)1);
    for (int i = 0; i < 5; i++)
        toggleClock();
    setTMS((uint8_t)0);
    usartSend("Done\n\r");
}

void writeInstruction();
