#include "avr/avr_uart.h"

void USART1_Init(uint16_t ubrr) {
  // Set Baud
  UBRR0H = (uint8_t)(ubrr >> 8) & 0xFF;
  UBRR0L = (uint8_t)(ubrr)&0xFF;
  UCSR0A = 0;
  // 8 data bits, 1 stop bit, 0 parity bits
  UCSR0C = (3 << UCSZ00);
  // Enable Tx and Rx
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void USART1_Tx(uint8_t data) {
  while (!(UCSR0A & (1 << UDRE0))) {
    // wait for Tx data register to empty
  }
  UDR0 = data;
}

uint8_t USART1_Rx(uint8_t *statusCounters) {
  while (!(UCSR0A & (1 << RXC0))) {
    // wait for Tx data register to empty
  }
  const uint8_t status = UCSR0A;
  if (statusCounters && status & (1 << FE0))
    statusCounters[0]++;
  if (statusCounters && status & (1 << DOR0))
    statusCounters[1]++;
  if (statusCounters && status & (1 << UPE0))
    statusCounters[2]++;
  return UDR0;
}
