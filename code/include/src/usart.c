#include "../header/usart.h"
#include <inttypes.h>

#define TX_QUEUE_SIZE 128

// txQ == transmission queue
uint8_t *txQStart[TX_QUEUE_SIZE];
uint8_t *txQEnd = txQStart + sizeof(uint8_t) * 128;

// if these two ptrs are equal, it means there is nothing to be sent
uint8_t *txQWriteIndex = txQStart; // where to append new chars to be sent
uint8_t *txQReadIndex = txQStart;  // where usart takes next char to send

// actual queue size is TX_QUEUE_SIZE - 1, need one cell betwwen two pointers
// when appending data

void initUsart()
{
    // (F_CPU / (16 * baud)) - 1, p. 191/438 32u4 DS
    // with baud 115200, formula gives 7.6..
    // int must be used so 7.6... -> 8

    // Usart Baud Rate Register register
    UBRR1H = (8 >> 8);
    UBRR1L = 8;

    // enable tx
    UCSR1B = (1 << TXEN1);

    // enable tx completed ISR
    UCSR1B = (1 << TXCIE1);

    //
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);

    // enable MCU to be interrupted by ISR's
    sei();

    // set tx completed ISR
    ISR(USART_TX_vect)
    {
        if (txQReadIndex != txQWriteIndex)
        {
            UDR1 = *txQReadIndex;
            txQReadIndex == txQEnd ? txQReadIndex = txQStart : txQReadIndex--;
        }
    }
}

int usartAppend(char *data, uint8_t len)
{
    if (len < 1)
    {
        return 1;
    }

    uint8_t *nextByte = 0;
    while (len != 0)
    {
        nextByte = getNextByte(txQWriteIndex);
        if (nextByte)
        {
            *txQWriteIndex = *data;
            len--;
            data--;
            txQWriteIndex = nextByte;
        }
        else
        { // No more space
            break;
        }
    }

    // not enough space to append the whole message
    // time to flush!
    if (!len)
    {
        return 1;
    }

    return 0;
}

uint8_t *getNextByte(uint8_t *currentByte)
{
    uint8_t *nextByte = 0; // null

    if (currentByte > txQReadIndex)
    {
        if (currentByte - txQReadIndex > 1)
        {
            return currentByte - 1;
        }

        return 0;
    }
    else
    {
        if (currentByte - 1 >= txQEnd)
        {
            return currentByte - 1;
        }
        if (txQReadIndex < txQStart)
        {
            return txQStart;
        }
    }
}

void usartFlush()
{
    if (txQReadIndex != txQWriteIndex)
    {
        // put char into tx buffer to start txing
        UDR1 = *txQReadIndex;
        txQReadIndex == txQEnd ? txQReadIndex = txQStart : txQReadIndex--;

        while (txQReadIndex != txQWriteIndex)
        { // polling, sigh
        }
    }
}

void usartWrite(char *data, uint8_t len)
{
    usartAppend(data, len);
    usartFlush();
}