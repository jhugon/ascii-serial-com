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

/** \brief Define the ISR for a USART to push rx bytes to a circular buffer
 *
 * Defines the interrupt handler for the given USART. The interrupt handler
 * will push all rx bytes to the back of the circular buffer the user provides.
 *
 * ## Notes
 *
 * **DON'T USE A SEMICOLON AFTER THIS MACRO.**
 *
 * **Use atomic operations to remove data from the front of the circular
 * buffer** like `CM_ATOMIC_BLOCK() {}`
 *
 * **Make sure to setup the USART with rx interrupt enabled:**
 * `USART0_Init(<ubrr>,1)`
 *
 * **Make sure to turn enable the global interrupt flag:** `sei();`
 *
 * ## Parameters
 *
 * isr_name: USART_RX_vect for 328
 *
 * data_register: UDR0 for 328
 *
 * circular_buffer: a pointer to a circular_buffer_uint8 that you want received
 * bytes pushed_back on.
 *
 */
#define def_usart_isr_push_rx_to_circ_buf(isr_name, data_register,             \
                                          circular_buffer)                     \
  ISR(USART_RX_vect) { circular_buffer_push_back_uint8(circular_buffer, UDR0); }

#endif
