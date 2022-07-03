#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "../header/jtag.h"
#include "../header/usart.h"

// GPIO direction registers
volatile uint8_t *dregb = (volatile uint8_t *)(0x04 + 0x20); // DDRB
volatile uint8_t *dregd = (volatile uint8_t *)(0x0A + 0x20); // DDRB

// write registers
volatile uint8_t *wregb = (volatile uint8_t *)(0x05 + 0x20); // PORTB
volatile uint8_t *wrebd = (volatile uint8_t *)(0x0B + 0x20); // PORTD

// read register
volatile uint8_t *rregb = (volatile uint8_t *)(0x03 + 0x20); // PINB
volatile uint8_t *rregd = (volatile uint8_t *)(0x09 + 0x20); // PIND

pin pins[AVAILABLE_PINS];
uint8_t jtagPins[4]; // eventually will be filled with TDI_ID, TDO_ID,.. etc defines
// the two arrays above are used to map a specific hw pin to a specific jtag signal
// for example, if jtagPins[0] = TDI_ID, it means that pins[0] viene usato come TDI

// these indeces represent the position of TDI_ID,.. etc inside jtagPins
uint8_t tdi_index = 1;
uint8_t tdo_index = 2;
uint8_t tms_index = 3;
uint8_t tck_index = 0;

void toggleClock()
{
    setRegister(pins[tck_index].wreg, pins[tck_index].number, HIGH);
    _delay_ms(1);
    setRegister(pins[tck_index].wreg, pins[tck_index].number, LOW);
    _delay_ms(1);
}

uint8_t isJtagEnabled()
{
    // TODO
    // read JTAGEN fuse

    return HIGH;
}

void setTDI(uint8_t state)
{
    setRegister(pins[tdi_index].wreg, pins[tdi_index].number, state);
    _delay_ms(1);
}

void setTMS(uint8_t state)
{
    setRegister(pins[tms_index].wreg, pins[tms_index].number, state);
    _delay_ms(1);
}

uint8_t getTDO()
{
    return *pins[tdo_index].rreg & (HIGH << pins[tdo_index].number) ? HIGH : LOW;
}

void initJtag()
{
    initHwPins();
    setJtagInterface();
    resetJtagFsm();
}

void initHwPins()
{
    // define all the hw pins available to jtag
    pins[0] = (pin){.dreg = dregd, .wreg = wrebd, .rreg = rregd, .number = 6}; // D6
    pins[1] = (pin){.dreg = dregb, .wreg = wregb, .rreg = rregb, .number = 7}; // B7
    pins[2] = (pin){.dreg = dregb, .wreg = wregb, .rreg = rregb, .number = 6}; // B6
    pins[3] = (pin){.dreg = dregb, .wreg = wregb, .rreg = rregb, .number = 5}; // B5
}

void setRegister(volatile uint8_t *reg, uint8_t number, uint8_t value)
{
    if (value)
        *reg |= (HIGH << number);
    else
        *reg &= ~(HIGH << number);
}

void setJtagInterface()
{
    jtagPins[tdi_index] = TDI_ID;
    jtagPins[tdo_index] = TDO_ID;
    jtagPins[tms_index] = TMS_ID;
    jtagPins[tck_index] = TCK_ID;

    // set pin direction based on jtag signal assigned
    for (uint8_t i = 0; i < AVAILABLE_PINS; i++)
    {
        // TDO is only input pin
        if (jtagPins[i] == TDO_ID)
            setRegister(pins[i].dreg, pins[i].number, LOW);
        else
        {
            setRegister(pins[i].dreg, pins[i].number, HIGH);
            setRegister(pins[i].wreg, pins[i].number, LOW); // ensure signals start LOW
        }
    }
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
    for (uint8_t i = 0; i < MAX_TAP_CHAIN_LENGTH * 2; i++)
        toggleClock();

    // set TDI HIGH and count how many
    // clocks it takes to show up on TDO
    setTDI(HIGH);
    for (uint8_t i = 0; i < MAX_TAP_CHAIN_LENGTH * 2; i++)
    {
        if (getTDO())
        {
            resetJtagFsm();
            return i;
        }
        toggleClock();
    }
    setTDI(LOW); // reset line

    resetJtagFsm();

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

    setTDI(LOW); // not needed, just to set a value

    // when in 'Test-Logic-Reset' state
    // the IDCODE instruction is in effect
    // just need to shift-out IDCODE from TDO (LSB first)
    moveFSM(LOW);  // go to 'Run-Test/Idle' state
    moveFSM(HIGH); // go to 'Select DR-Scan'
    moveFSM(LOW);  // go to 'Capture-DR'
    moveFSM(LOW);  // go to 'Shift-DR'

    uint32_t idcode = 0;
    // for each TAP in chain
    for (uint8_t i = tapChainLen; i > 0; i--)
    {
        // shift out IDCODE and store it
        for (uint8_t j = 0; j < IDCODE_LENGTH; j++)
        {
            idcode |= (uint32_t)getTDO() << j; // important to cast!
            toggleClock();
        }

        // print out found ID
        usartSend("TAP %d IDCODE: 0x%X = ", i, idcode);
        for (int8_t i = 31; i >= 0; i--)
            (idcode & ((uint32_t)1 << i)) ? usartSend("1") : usartSend("0");
        usartSend("\n\r");
        usartSend("Manufacturer ID: 0x%X = | ", (idcode << 20) >> 21);
        usartSend("Part Number: 0x%X | ", (idcode << 4) >> 16);
        usartSend("Version: 0x%X\n\r", idcode >> 28);
        usartSend("-----\n\r");
    }

    resetJtagFsm();
}

uint8_t findJtagInterface()
{
    for (uint8_t _tdi = 0; _tdi < 4; _tdi++)
    {
        jtagPins[_tdi] = TDI_ID;
        tdi_index = _tdi;
        for (uint8_t _tdo = 0; _tdo < 4; _tdo++)
        {
            if (_tdo != _tdi)
            {
                jtagPins[_tdo] = TDO_ID;
                tdo_index = _tdo;
                for (uint8_t _tms = 0; _tms < 4; _tms++)
                {
                    if (_tms != _tdi && _tms != _tdo)
                    {
                        jtagPins[_tms] = TMS_ID;
                        tms_index = _tms;
                        for (uint8_t _tck = 0; _tck < 4; _tck++)
                        {
                            if (_tck != _tdi && _tck != _tdo && _tck != _tms)
                            {
                                jtagPins[_tck] = TCK_ID;
                                tck_index = _tck;

                                usartSend("Trying [");
                                for (uint8_t t = 0; t < AVAILABLE_PINS; t++)
                                {
                                    switch (jtagPins[t])
                                    {
                                    case TDI_ID:
                                        usartSend("TDI");
                                        if (t != AVAILABLE_PINS - 1)
                                            usartSend(", ");
                                        break;
                                    case TDO_ID:
                                        usartSend("TDO");
                                        if (t != AVAILABLE_PINS - 1)
                                            usartSend(", ");
                                        break;
                                    case TMS_ID:
                                        usartSend("TMS");
                                        if (t != AVAILABLE_PINS - 1)
                                            usartSend(", ");
                                        break;
                                    case TCK_ID:
                                        usartSend("TCK");
                                        if (t != AVAILABLE_PINS - 1)
                                            usartSend(", ");
                                        break;
                                    }
                                }
                                usartSend("]...");
                                setJtagInterface();

                                if (getTapChainLenght())
                                {
                                    usartSend("Success!\n\r");
                                    usartSend("Signal | Pin\n\r");
                                    usartSend(" TDI   | %d\n\r", tdi_index);
                                    usartSend(" TDO   | %d\n\r", tdo_index);
                                    usartSend(" TMS   | %d\n\r", tms_index);
                                    usartSend(" TCK   | %d\n\r", tck_index);

                                    return HIGH;
                                }
                                else
                                    usartSend("Fail.\n\r");
                            }
                            resetJtagFsm();
                        }
                    }
                }
            }
        }
    }

    usartSend("No JTAG interface found.\n\r");
    return LOW;
}

void resetJtagFsm()
{
    // clock TMS HIGH for 5 cycles to put FSM
    // into 'Test-Logic-Reset' state
    setTDI(LOW); // for safety set TDI to 0
    setTMS(HIGH);
    for (int i = 0; i < 5; i++)
        toggleClock();
    setTMS(LOW);
}
