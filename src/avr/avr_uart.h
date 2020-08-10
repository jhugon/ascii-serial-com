#ifndef AVR_UART_H
#define AVR_UART_H

#include <avr/io.h>
#include <stdint.h>

/** \brief Initialize USART1
 *
 *  \param ubrr: 12 bits: should be clock / 16 / baud - 1
 *
 */
void USART1_Init(uint16_t ubrr);

/** \brief Transmit data with USART1 (blocking)
 *
 *  \param data: data to transmit
 *
 */
void USART1_Tx(uint8_t data);

/** \brief Receive data with USART1 (blocking)
 *
 *  \param statusCounters: should be NULL or a pointer to a 3 element array
 *      The 0 element will be the number of frame errors
 *          1 element will be the number of data overrun errors
 *          2 element will be the number of parity errors
 *
 *  \return the received data
 *
 */
uint8_t USART1_Rx(uint8_t *statusCounters);

#endif
