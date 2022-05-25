#include <inttypes.h>

#define TX_QUEUE_SIZE 128 // bytes

void initUsart();

void usartWrite(char *data, uint8_t len);

int usartAppend(char *data, uint8_t len);

void usartFlush();

uint8_t addCharTxBuffer();

uint8_t *getNextByte(uint8_t *currentByte);