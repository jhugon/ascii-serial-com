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
 * **Make sure to setup the USART and run both**
 * `nvic_enable_irq(NVIC_<usart>_IRQ);` and
 * `usart_enable_rx_interrupt(<usart>);`
 *
 * **Not sure if this works with UARTs or LPUARTs**
 *
 * ## Parameters
 *
 * isr_name: usart1_isr, usart2_isr, ...
 *
 * usart: the USART you want to use like USART1, USART2, ...
 *
 * circular_buffer: a pointer to a circular_buffer_uint8 that you want received
 * bytes pushed_back on.
 *
 */
#define def_usart_isr_push_rx_to_circ_buf(isr_name, usart, circular_buffer)    \
  _Pragma("GCC diagnostic ignored \"-Wshadow\"")                               \
      _Pragma("GCC diagnostic push")                                           \
          _Pragma("GCC diagnostic ignored \"-Wunused-function\"")              \
              _Pragma("GCC diagnostic push") void                              \
              isr_name(void) {                                                 \
    _Pragma("GCC diagnostic pop") _Pragma(                                     \
        "GCC diagnostic pop") if (((USART_CR1(usart) & USART_CR1_RXNEIE) !=    \
                                   0) &&                                       \
                                  ((USART_ISR(usart) & USART_ISR_RXNE) !=      \
                                   0)) {                                       \
      circular_buffer_push_back_uint8(circular_buffer,                         \
                                      usart_recv(usart) & 0xFF);               \
    }                                                                          \
  }

#endif
