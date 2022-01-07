#ifndef STM_USART_H
#define STM_USART_H

/** \file
 *
 * \brief To be used with the USART peripherals on STM32 microcontrollers
 *
 * */

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdint.h>

/** \brief Setup a USART
 *
 * Setup a USART in async, 8 bit, 1 stop bit, no parity, no flow-control, tx-rx
 * mode.
 *
 * ## Notes
 *
 * **The user must setup the peripheral clocks for both this USART and the
 * GPIO input/outpu port(s).**
 *
 * **The user must enable the USART when ready with:**
 * `usart_enable(usart);`
 *
 * **This macro takes care of setting up the GPIO pins.**
 *
 * **Not sure if this works with UARTs or LPUARTs**
 *
 * ## Parameters
 *
 * usart: the USART you want to use like USART1, USART2, ...
 *
 * baud: the baud rate of the USART, uint32_t
 *
 * tx_port: GPIOA, GPIOB, ...
 *
 * tx_pin: GPIO0, GPIO1, ...
 *
 * tx_af: alternate function for the given pin/port to hook it up to the USART,
 * e.g. GPIO_AF0, GPIO_AF1, ...
 *
 * rx_port: GPIOA, GPIOB, ...
 *
 * rx_pin: GPIO0, GPIO1, ...
 *
 * rx_af: alternate function for the given pin/port to hook it up to the USART,
 * e.g. GPIO_AF0, GPIO_AF1, ...
 *
 */
#define setup_usart(usart, baud, tx_port, tx_pin, tx_af, rx_port, rx_pin,      \
                    rx_af)                                                     \
  gpio_mode_setup(tx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, tx_pin);              \
  gpio_set_af(tx_port, tx_af, tx_pin);                                         \
                                                                               \
  gpio_mode_setup(rx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, rx_pin);              \
  gpio_set_af(rx_port, rx_af, rx_pin);                                         \
                                                                               \
  usart_set_baudrate(usart, baud);                                             \
  usart_set_databits(usart, 8);                                                \
  usart_set_parity(usart, USART_PARITY_NONE);                                  \
  usart_set_stopbits(usart, USART_CR2_STOPBITS_1);                             \
  usart_set_mode(usart, USART_MODE_TX_RX);                                     \
  usart_set_flow_control(usart, USART_FLOWCONTROL_NONE)

#endif
