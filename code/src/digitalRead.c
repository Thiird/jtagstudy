// these two defines must go before include of avr/io.h!!
#define F_CPU 16000000UL // 16MHz
#define __AVR_ATmega32U4__

#include <avr/io.h>
#include <util/delay.h>

#define SAMPLE_PIN DDD6

int main(void)
{
    // set pins direction
    DDRC |= (1 << PORTC7);      // led output
    DDRD &= ~(1 << SAMPLE_PIN); // input pin

    while (1)
    {
        if (PIND & (1 << SAMPLE_PIN))
        {
            PORTC |= (1 << PORTC7);
        }
        else
        {
            PORTC &= ~(1 << PORTC7);
        }
        _delay_ms(10);
    }

    return 0;
}