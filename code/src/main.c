// these two defines must go before include of avr/io.h!!
#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <unistd.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "../include/header/jtag.h"
#include "../include/header/usart.h"

int main(int argc, char **argv)
{
    DDRC |= (1 << PORTC7); // led output

    char usartBuffer[128];

    initUsart();

    // set jtag pins IO direction
    DDRD |= (1 << TDI);  // output
    DDRB &= ~(1 << TDO); // input
    DDRB |= (1 << TMS);  // output
    DDRB |= (1 << TCK);  // output

    // 11011000

    if (FUSE_JTAGEN)
    {
        snprintf(usartBuffer, 11, "JTAGEN: 1\n\r");
    }
    else
    {
        snprintf(usartBuffer, 11, "JTAGEN: 0\n\r");
    }

    usartWrite(&usartBuffer[0]);

    while (1)
    {
        _delay_ms(1000);
    }

    return 0;
}