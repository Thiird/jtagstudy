#ifndef __AVR_ATmega32U4__
#define __AVR_ATmega32U4__
#endif

#define F_CPU 16000000
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../header/usart.h"

// txQ == transmission queue
uint8_t txQueue[TX_QUEUE_SIZE];

uint8_t *txQStart = txQueue;
uint8_t *txQEnd = txQueue + (TX_QUEUE_SIZE - 1);

// if these two ptrs are equal, it means there is nothing to be sent
uint8_t *txQWriteIndex = txQueue; // where to append new chars to be sent
uint8_t *txQReadIndex = txQueue;  // where usart takes next char to send

// actual queue size is TX_QUEUE_SIZE - 1, need one cell betwwen
// read/write ptrs when appending data

char usartBuffer[128];

void initUsart()
{
    // UBBRn -> Usart Baud Rate Register register
    // UCSRnB/C -> USART Control State Register B/C

    // UBRR = (F_CPU / (16 * baud)) - 1, p. 191/438 32u4 DS
    // with baud 115200, formula gives 7.6..
    // int must be used so 7.6... -> 8

    // set baud rate
    UBRR1H = (8 >> 8);
    UBRR1L = 8;

    // enable tx and tx completed ISR
    UCSR1B = (1 << TXEN1) | (1 << TXCIE1);

    // set char size to 8 bit
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);

    // enable MCU to be interrupted by ISR's
    sei();
}

ISR(USART1_TX_vect)
{
    addCharTxBuffer();
}

void usartSend(char *str, ...)
{
    uint8_t strLen = 0;
    uint8_t *tempPtr = str;

    while (*tempPtr != '\0')
    {
        strLen++;
        tempPtr++;
    }

    if (!strLen)
        return;

    va_list args;
    va_start(args, str);
    vsnprintf(usartBuffer, 128, str, args);
    usartWrite(&usartBuffer[0]);
    va_end(args);
}

// takes in a null terminated ('\0') string pointer
void usartWrite(char *str)
{
    // double ptr is needed to keep track
    // of the already appended string while flushing

    char **data = &str;
    while (usartAppend(data))
    {
        usartFlush();
    }

    usartFlush();
}

uint8_t usartAppend(char **data)
{
    uint8_t *nextQByte = 0;
    while (1)
    {
        if (**data == '\0')
            return 0;

        nextQByte = getNextQByte(txQWriteIndex);
        if (nextQByte)
        {
            *txQWriteIndex = **data;
            (*data)++;
            txQWriteIndex = nextQByte;
        }
        else
        { // no more space, time to flush
            return (uint8_t)1;
        }
    }
}

void usartFlush()
{
    if (txQReadIndex != txQWriteIndex)
    {
        // start txing
        addCharTxBuffer();

        while (txQReadIndex != txQWriteIndex)
        { // polling, sigh
          // waiting for USART_TX_vect ISR to send all the queued data
        }
    }
}

void addCharTxBuffer()
{
    if (txQReadIndex != txQWriteIndex)
    {
        UDR1 = *txQReadIndex;
        txQReadIndex == txQEnd ? txQReadIndex = txQStart : txQReadIndex++;
    }
}

uint8_t *getNextQByte(uint8_t *currentByte)
{
    if (currentByte == txQReadIndex)
    {
        if (currentByte == txQEnd)
            return txQStart;
        else
            return currentByte + 1;
    }

    if (currentByte < txQReadIndex)
    {
        if (txQReadIndex - currentByte > 1)
            return currentByte + 1;

        // must leave an empty cell between write and read indices
        // when writing, flush() knows there are chars to send if
        // the two are not equal

        return (uint8_t)0;
    }

    // if currentByte > txQReadIndex

    if (txQEnd - currentByte >= 1)
        return currentByte + 1;

    if (currentByte == txQEnd)
    {
        if (txQReadIndex > txQStart)
            return txQStart;

        return (uint8_t)0;
    }
}