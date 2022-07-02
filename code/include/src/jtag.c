#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "../header/jtag.h"
#include "../header/usart.h"

#define TDI_ID 0
#define TDO_ID 1
#define TMS_ID 2
#define TCK_ID 3

#define AVAILABLE_PINS 4

volatile uint8_t *ddrd = (volatile uint8_t *)(0x0A + 0x20);
volatile uint8_t *ddrb = (volatile uint8_t *)(0x04 + 0x20);

volatile uint8_t *portd = (volatile uint8_t *)(0x0B + 0x20);
volatile uint8_t *portb = (volatile uint8_t *)(0x05 + 0x20);

typedef struct
{
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t number;
} pin;

pin pins[AVAILABLE_PINS];
uint8_t jtagPins[4] = {TDI_ID, TDO_ID, TMS_ID, TCK_ID};
// the two arrays above are used to map a specific hw pin to a specific jtag signal
// for example, if jtagPins[0] = TDI_ID, it means that pins[0] viene usato come TDI

// these indeces represent the position of TDI_ID, etc inside jtagPins
uint8_t tdi_index = 0;
uint8_t tdo_index = 1;
uint8_t tms_index = 2;
uint8_t tck_index = 3;

void toggleClock()
{
    setRegister(pins[tck_index].port, pins[tck_index].number, HIGH);
    _delay_ms(1);
    setRegister(pins[tck_index].port, pins[tck_index].number, LOW);
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
    if (state)
        setRegister(pins[tdi_index].port, pins[tdi_index].number, HIGH);
    else
        setRegister(pins[tdi_index].port, pins[tdi_index].number, LOW);
    _delay_ms(1);
}

void setTMS(uint8_t state)
{
    if (state)
        setRegister(pins[tms_index].port, pins[tms_index].number, HIGH);
    else
        setRegister(pins[tms_index].port, pins[tms_index].number, LOW);
    _delay_ms(1);
}

void initHwPins()
{
    *pins = *(pin[AVAILABLE_PINS]){
        [0] = {.ddr = ddrd, .port = portd, .number = 6},  // D6
        [1] = {.ddr = ddrb, .port = portb, .number = 7},  // B7
        [2] = {.ddr = ddrb, .port = portb, .number = 6},  // B6
        [3] = {.ddr = ddrb, .port = portb, .number = 5}}; // B5
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
    // set pin direction based on jtag signal assigned
    for (uint8_t i = 0; i < AVAILABLE_PINS; i++)
    {
        // TDO is only input pin
        if (jtagPins[i] == TDO_ID)
            setRegister(pins[i].ddr, pins[0].number, 0);
        else
        {
            setRegister(pins[i].ddr, pins[0].number, 1);
            setRegister(pins[i].port, pins[0].number, 0); // ensure signals start LOW
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
        usartSend("-----\n\r");
    }

    resetJtagFsm(); // go to 'Test-Logic-Reset' state
}

void findJtagInterface()
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

                                setJtagInterface();
                                if (getTapChainLenght())
                                {
                                    usartSend("Interface found!\n\r");
                                    /*usartSend("TDI: %d\n\r", indeces[0]);
                                    usartSend("TDO: %d\n\r", indeces[1]);
                                    usartSend("TMS: %d\n\r", indeces[2]);
                                    usartSend("TCK: %d\n\r", indeces[3]);*/
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }

        usartSend("No JTAG interface found\n\r");
        return;
    }

    void resetJtagFsm();

    // clock TMS HIGH for 5 cycles to reset FSM
    // into 'Test-Logic-Reset' state
    setTDI(LOW); // for safety set TDI to 0
    setTMS(HIGH);
    for (int i = 0; i < 5; i++)
        toggleClock();
    setTMS(LOW);
}