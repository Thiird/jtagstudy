#include <inttypes.h>

#define TX_QUEUE_SIZE 128 // bytes

void initUsart();

void usartSend(char *str, ...);

void usartWrite(char *data);

uint8_t usartAppend(char **data);

void usartFlush();

void addCharTxBuffer();

uint8_t *getNextQByte(uint8_t *currentByte);
