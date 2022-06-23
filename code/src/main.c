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
    DDRD &= ~(1 << PD1); // button input pin
    DDRC |= (1 << PC7);  // LED output pin

    uint8_t sent = 0;

    initUsart();
    initJtagInterface();

    while (1)
    {
        if (PIND & (1 << PD1))
        {
            if (!sent)
            {
                PORTC |= (1 << PORTC7);

                resetJtagFsm();
                usartSend("TAP chain lenght is: %d\n\r", countTapChainLenght());
                usartSend("=======\n\r");

                sent = 1;
                PORTC &= ~(1 << PORTC7); // turn off led
            }
        }
        else
        {
            sent = 0;
            PORTC &= ~(1 << PORTC7); // turn off led
        }
    }

    return 0;
}