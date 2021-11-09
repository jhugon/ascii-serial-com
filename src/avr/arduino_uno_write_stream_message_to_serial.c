#include "avr/avr_uart.h"
#include <avr/io.h>

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

#define bufCap 64

static const char message[] = ">00s0\n";
static const uint8_t message_len = 6;
uint8_t iChar = 0;

int main(void) {

  USART0_Init(MYUBRR, 0);

  while (1) {
    USART0_Tx(message[iChar]);
    iChar++;
    if (iChar == message_len) {
      iChar = 0;
    }
  }
  return 0;
}
