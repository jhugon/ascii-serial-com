#include "arm/stm_usart.h"
#include <libopencm3/stm32/rcc.h>

// USART2 should be connected through USB
#define ASC_USART USART2

uint8_t tmp_byte = 0;

int main(void) {

  // USART2 TX: PA2 AF1
  // USART2 RX: PA3 AF1
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART2);
  setup_usart(ASC_USART, 9600, GPIOA, GPIO2, GPIO_AF1, GPIOA, GPIO3, GPIO_AF1);
  usart_enable(ASC_USART);

  while (1) {
    if ((USART_ISR(ASC_USART) & USART_ISR_RXNE)) {
      tmp_byte = (uint8_t)usart_recv(ASC_USART) & 0xFF;
    }
    if (tmp_byte && (USART_ISR(ASC_USART) & USART_ISR_TXE)) {
      usart_send(ASC_USART, tmp_byte);
      tmp_byte = 0;
    }
  }

  return 0;
}
