#include "avr/avr_uart.h"
#include <avr/io.h>

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

uint8_t byteBuffer;

int main(void) {

  USART1_Init(MYUBRR);

  while (1) {
    USART1_Rx(byteBuffer);
    USART1_Tx(byteBuffer);
  }
  return 0;
}
