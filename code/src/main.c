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
    // set jtag pins IO direction
    DDRD |= (1 << TDI);  // output
    DDRB &= ~(1 << TDO); // input
    DDRB |= (1 << TMS);  // output
    DDRB |= (1 << TCK);  // output

    initUsart();

    char usartBuffer[128];
    uint8_t cont = 0;

    while (1)
    {
        usartWrite(&usartBuffer[0]);
        cont++;
        snprintf(usartBuffer, 15, "Number is %d\n\r", cont);
        _delay_ms(100);
    }

    return 0;
}