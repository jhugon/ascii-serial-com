#include "avr/avr_uart.h"
#include <avr/io.h>

#define F_CPU 16000000L
#include <util/delay.h>
#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

uint8_t byteBuffer;

int main(void) {

  USART0_Init(MYUBRR, 0);

  byteBuffer = 48;
  while (1) {
    USART0_Tx(byteBuffer);
    byteBuffer++;
    if (byteBuffer > 57) {
      byteBuffer = 48;
      //_delay_ms(100);
    }
  }
  return 0;
}
