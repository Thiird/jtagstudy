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
    uint8_t idcode = 0;
    while (1)
    {
        if (PIND & (1 << PD1))
        {
            if (!sent)
            {
                PORTC |= (1 << PORTC7);

                idcode |= (uint32_t)0 << 0;
                for (uint8_t f = 31; f > 0; f--)
                {
                    usartSend("%d", (idcode & (uint32_t)1 << f) ? 1 : 0);
                    //_delay_ms(1);
                }
                usartSend("\n\r");

                // usartSend("%d TAP's detected.\n\r", getTapChainLenght());
                // getDeviceIds();
                sent = 1;
                usartSend("====END====\n\r");
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