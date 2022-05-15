#include <avr/io.h>
#include <avr/interrupt.h>

#include "include/gpio.h"

#define MHZ 1000000
#define F_CPU 16 * MHZ

ISR(TIMER1_OVF_vect)
{
    PORTC ^= _BV(PORTC7);
    TCNT1 = 65535 - (F_CPU / 1024);
}

int main(int argc, char **argv)
{
    // set led pin C7 as output
    DDRC |= _BV(DDC7);

    // max value is 65535
    TCNT1 = 0;

    // set prescaler to 1024
    // 16Mhz/1024 = 15625Hz
    TCCR1B = (1 << CS10) | (1 << CS12);

    // set to be an overflow timer
    TCCR1A = 0x00;

    TIMSK1 = (1 << TOIE1);

    // make cpu able to be interrupted by an isr
    sei();

    while (1)
    {
    }

    return 0;
}