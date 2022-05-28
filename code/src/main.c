// these two defines must go before include of avr/io.h!!
#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <unistd.h>
#include <avr/io.h>
#include <util/delay.h>

#include "../include/header/jtag.h"
#include "../include/header/usart.h"

int main(int argc, char **argv)
{
    DDRC |= (1 << PORTC7); // led output

    initUsart();

    // set jtag pins IO direction
    DDRD |= (1 << TDI);  // output
    DDRB &= ~(1 << TDO); // input
    DDRB |= (1 << TMS);  // output
    DDRB |= (1 << TCK);  // output

    if (!isJtagEnabled())
    {
        usartSend("Jtag is not enabled! Set JTAGEN fuse bit to 0 first.\n\r\0");
        return 1;
    }

    resetJtagFsm();
    /*int chainLength = countTapChainLenght();
    usartSend("TAP chain length is: %d\n\r\0");*/

    while (1)
    {
        usartSend("==========\n\r\0");
        usartSend("TDO: %d\n\r", getTDO());
        usartSend("==========\n\r\0");
        _delay_ms(1000);
    }

    return 0;
}