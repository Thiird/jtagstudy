// these two defines must go before include of avr/io.h!!
#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000

#include <unistd.h>
#include <avr/io.h>

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

    char str[220] =
        "Ciao, con la seguente volevo dirti che insomma io ti amo da pazzi e mi piaci tanto, elon vuoi sposarmi Ciao, con la seguente volevo dirti che insomma io ti amo da pazzi e mi piaci tanto, elon vuoi sposarmiaaaaaaaaaa\n\r\0";

    usartWrite(&str[0]);

    while (1)
    {
    }

    return 0;
}