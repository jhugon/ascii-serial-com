#ifndef AVR_UART_H
#define AVR_UART_H

/** \file */

#include <avr/io.h>
#include <stdint.h>

// Like bools
#define USART0_can_write_Tx_data (UCSR0A & (1 << UDRE0))
#define USART0_can_read_Rx_data (UCSR0A & (1 << RXC0))

// Enable/disable USART_UDRE_vect interrupt that triggers when USART (transmit)
// data register is empty i.e. ready for more data
#define USART0_enable_udre_interrupt UCSR0B |= (1 << UDRIE0)
#define USART0_disable_udre_interrupt UCSR0B &= ~(1 << UDRIE0)

/** \brief Initialize USART0
 *
 *  8 bit, 1 stop bit, no parity bit
 *
 *  \param ubrr: 12 bits of uint16_t: should be clock / 16 / baud - 1
 *
 *  \param rxIntEnable: 1 bit enable RX interrupt
 *
 */
#define USART0_Init(ubrr, rxIntEnable)                                         \
  UBRR0H = (uint8_t)(ubrr >> 8) & 0xFF;                                        \
  UBRR0L = (uint8_t)(ubrr)&0xFF;                                               \
  UCSR0A = 0;                                                                  \
  UCSR0C = (3 << UCSZ00);                                                      \
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (rxIntEnable << RXCIE0);

/** \brief Transmit data with USART0 (blocking)
 *
 *  \param data: data to transmit uint8_t
 *
 */
#define USART0_Tx(data)                                                        \
  while (!USART0_can_write_Tx_data) {                                          \
  }                                                                            \
  UDR0 = data;

/** \brief Receive data with USART0 (blocking)
 *
 * \param destVar: uint8_t place to copy the data
 *
 */
#define USART0_Rx(destVar)                                                     \
  while (!USART0_can_read_Rx_data) {                                           \
  }                                                                            \
  destVar = UDR0;

#endif
