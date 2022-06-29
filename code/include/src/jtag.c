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
    _delay_ms(1);
    PORTB &= ~(1 << TCK);
    _delay_ms(1);
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
    _delay_ms(1);
}

void setTMS(uint8_t state)
{
    if (state)
        PORTB |= (1 << TMS);
    else
        PORTB &= ~(1 << TMS);
    _delay_ms(1);
}

void initJtagInterface()
{
    // usartSend("Init JTAG...");
    DDRD |= (1 << TDI);  // output
    DDRB &= ~(1 << TDO); // input
    DDRB |= (1 << TMS);  // output
    DDRB |= (1 << TCK);  // output

    // ensure TCK starts LOW
    PORTB &= ~(1 << TCK);

    resetJtagFsm();
    // usartSend("Done.\n\r");
}

void moveFSM(uint8_t bit)
{
    setTMS(bit);
    toggleClock();
}

uint8_t getTapChainLenght()
{
    moveFSM(LOW);  // moving to 'Run-Test/Idle' state
    moveFSM(HIGH); // moving to 'Select DR-Scan' state
    moveFSM(HIGH); // moving to 'Select IR-Scan' state
    moveFSM(LOW);  // moving to 'Capture IR' state
    moveFSM(LOW);  // moving to 'Shift IR' state

    // shift in BYPASS instruction (all 1's)
    // there could be many different devices in the TAP chain
    // for which the IR length is unknown, so send a bunch
    // of 1's to fill all the IR's
    setTDI(HIGH);
    for (uint16_t i = 0; i < 1000; i++)
        toggleClock();
    // last bit of instruction must have TMS high to exit 'shift-IR' state
    moveFSM(HIGH);
    setTDI(LOW);

    // now in 'Exit1-IR' state, go to 'Shift-DR' state

    moveFSM(HIGH); // go to 'Update IR' state, after this, loaded instruction is operational
    // ready to shift in the test bits into TDI and check
    // offset on TDO, which will determine length of TAP chain
    // in BYPASS mode, each TAP will delay TDO in respect
    // to TDI by exactly one clock
    moveFSM(HIGH); // go to 'Select DR-Scan' state
    moveFSM(LOW);  // go to 'Capture-DR' state
    moveFSM(LOW);  // go to 'Shift-DR' state

    // send plenty of 0's to flush the BYPASS chain
    setTDI(LOW);
    for (uint8_t i = 0; i < MAX_TAP_CHAIN_LENGTH; i++)
        toggleClock();

    // set TDI HIGH and count how many
    // clocks it takes to show up on TDO
    setTDI(HIGH);
    for (uint8_t i = 0; i < MAX_TAP_CHAIN_LENGTH; i++)
    {
        if (getTDO())
        {
            resetJtagFsm(); // go to 'Test-Logic-Reset' state
            return i;
        }

        toggleClock();
    }
    setTDI(LOW); // reset line

    resetJtagFsm(); // go to 'Test-Logic-Reset' state

    return LOW;
}

void getDeviceIds()
{
    /*

            MSB                                                            LSB
             +-----------+---------------+--------------------+-------------+
    IDCODE = |  Version  |  Part Number  |  Manufacturer ID   |  Fixed (1)  |
             +-----------+---------------+--------------------+-------------+
                 31...28      27...12            11...1              0
    */

    uint8_t tapChainLen = getTapChainLenght();
    usartSend("%d TAPs detected.\n\r", tapChainLen);

    setTDI(LOW); // ignored

    // when in 'Test-Logic-Reset' state
    // the IDCODE instruction is in effect
    // just need to shift-out IDCODE from TDO (LSB first)
    moveFSM(LOW);  // go to 'Run-Test/Idle' state
    moveFSM(HIGH); // go to 'Select DR-Scan'
    moveFSM(LOW);  // go to 'Capture-DR'
    moveFSM(LOW);  // go to 'Shift-DR'

    uint32_t idcode = 0;
    uint8_t bit = 0;
    // for each TAP in chain
    for (uint8_t i = tapChainLen; i > 0; i--)
    {
        // shift out IDCODE and store it
        for (uint8_t j = 0; j < IDCODE_LENGTH; j++)
        {
            idcode |= (uint32_t)getTDO() << j;
            toggleClock();
        }

        usartSend("TAP %d IDCODE: 0x%X = ", i, idcode);
        for (int8_t i = 31; i >= 0; i--)
            (idcode & ((uint32_t)1 << i)) ? usartSend("1") : usartSend("0");
        usartSend("\n\r");
        usartSend("Manufacturer ID: 0x%X = | ", (idcode << 20) >> 21);
        usartSend("Part Number: 0x%X | ", (idcode << 4) >> 16);
        usartSend("Version: 0x%X\n\r", idcode >> 28);
    }

    resetJtagFsm(); // go to 'Test-Logic-Reset' state
}

void resetJtagFsm()
{
    // clock TMS HIGH for 5 cycles to reset FSM
    // into 'Test-Logic-Reset' state
    setTDI(LOW); // for safety set TDI to 0
    setTMS(HIGH);
    for (int i = 0; i < 5; i++)
        toggleClock();
    setTMS(LOW);
}