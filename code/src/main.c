// these two defines must go before include of avr/io.h!!
#define __AVR_ATmega32U4__
#define F_CPU 16000000

#include <unistd.h>
#include <avr/io.h>
#include <util/delay.h>

#include "include/header/jtag.h"
#include "include/header/usart.h"

int main(int argc, char **argv)
{
    initUsart(115200);

    // set jtag pins IO direction
    DDRD |= (1 << TDI);  // output
    DDRB &= ~(1 << TDO); // input
    DDRB |= (1 << TMS);  // output
    DDRB |= (1 << TCK);  // output

    resetJtag();
    serialWrite("TAP chain length: %d\n", countDevices());

    return 0;
}