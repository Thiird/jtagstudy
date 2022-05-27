#include <inttypes.h>

#define TX_QUEUE_SIZE 128 // bytes

void initUsart();

void usartWrite(char *data);

uint8_t usartAppend(char **data);

void usartFlush();

uint8_t addCharTxBuffer();

uint8_t *getNextQByte(uint8_t *currentByte);
