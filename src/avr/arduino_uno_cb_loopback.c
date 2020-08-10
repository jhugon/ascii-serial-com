#include "avr/avr_uart.h"
#include "circular_buffer.h"
#include <avr/io.h>

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

#define bufCap 64

uint8_t byteBuffer;
uint8_t rawBuffer[bufCap];
circular_buffer_uint8 cb;

int main(void) {

  circular_buffer_init_uint8(&cb, bufCap, rawBuffer);

  USART0_Init(MYUBRR);

  while (1) {
    if (USART0_can_read_Rx_data) {
      circular_buffer_push_back_uint8(&cb, UDR0);
    }
    if (circular_buffer_get_size_uint8(&cb) > 0 && USART0_can_write_Tx_data) {
      UDR0 = circular_buffer_pop_front_uint8(&cb);
    }
  }
  return 0;
}
