#ifndef AVR_UART_H
#define AVR_UART_H

#include <avr/io.h>
#include <stdint.h>

/** \brief Initialize USART1
 *
 *  8 bit, 1 stop bit, no parity bit
 *
 *  \param ubrr: 12 bits of uint16_t: should be clock / 16 / baud - 1
 *
 */
#define USART1_Init(ubrr)                                                      \
  UBRR0H = (uint8_t)(ubrr >> 8) & 0xFF;                                        \
  UBRR0L = (uint8_t)(ubrr)&0xFF;                                               \
  UCSR0A = 0;                                                                  \
  UCSR0C = (3 << UCSZ00);                                                      \
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);

/** \brief Transmit data with USART1 (blocking)
 *
 *  \param data: data to transmit uint8_t
 *
 */
#define USART1_Tx(data)                                                        \
  while (!(UCSR0A & (1 << UDRE0))) {                                           \
  }                                                                            \
  UDR0 = data;

/** \brief Receive data with USART1 (blocking)
 *
 * \param destVar: uint8_t place to copy the data
 *
 */
#define USART1_Rx(destVar)                                                     \
  while (!(UCSR0A & (1 << RXC0))) {                                            \
  }                                                                            \
  destVar = UDR0;

#endif
