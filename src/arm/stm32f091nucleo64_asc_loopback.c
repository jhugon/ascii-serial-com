#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "circular_buffer.h"

char dataBuffer[MAXDATALEN];
ascii_serial_com asc;

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;
char ascVersion, appVersion, command;
size_t dataLen;

uint16_t nExceptions;

// USART2 should be connected through USB
// USART2 TX: PA2 AF1
// USART2 RX: PA3 AF1
static void usart_setup(void) {
  rcc_periph_clock_enable(RCC_USART2);
  rcc_periph_clock_enable(RCC_GPIOA);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);

  gpio_set_af(GPIOA, GPIO_AF1, GPIO2);
  gpio_set_af(GPIOA, GPIO_AF1, GPIO3);

  nvic_enable_irq(NVIC_USART2_IRQ);

  usart_set_baudrate(USART2, 9600);
  usart_set_databits(USART2, 8);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_stopbits(USART2, USART_CR2_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX_RX);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  usart_enable_rx_interrupt(USART2);

  usart_enable(USART2);
}

uint8_t tmp_byte = 0;

int main(void) {

  nExceptions = 0;

  ascii_serial_com_init(&asc);
  ascii_serial_com_set_ignore_CRC_mismatch(&asc);
  circular_buffer_uint8 *asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

  circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                             extraInputBuffer_raw);

  usart_setup();

  while (1) {
    Try {
      // Move data from extraInputBuffer to asc_in_buf
      if (!circular_buffer_is_empty_uint8(&extraInputBuffer)) {
        CM_ATOMIC_BLOCK() {
          tmp_byte = circular_buffer_pop_front_uint8(&extraInputBuffer);
        }
        circular_buffer_push_back_uint8(asc_in_buf, tmp_byte);
      }

      // Process in messages and loop them back to out
      if (!circular_buffer_is_empty_uint8(asc_in_buf)) {
        ascii_serial_com_get_message_from_input_buffer(
            &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
        if (command != '\0') {
          ascii_serial_com_put_message_in_output_buffer(
              &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
        }
      }

      // Write data from asc_out_buf to serial
      if (!circular_buffer_is_empty_uint8(asc_out_buf) &&
          (USART_ISR(USART2) & USART_ISR_TXE)) {
        tmp_byte = circular_buffer_pop_front_uint8(asc_out_buf);
        usart_send(USART2, tmp_byte);
      }
    }
    Catch(e) { nExceptions++; }
  }

  return 0;
}

#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic push
void usart2_isr(void) {
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
  static uint16_t isr_tmp_byte;
  if (((USART_CR1(USART2) & USART_CR1_RXNEIE) != 0) &&
      ((USART_ISR(USART2) & USART_ISR_RXNE) != 0)) {
    isr_tmp_byte = usart_recv(USART2) & 0xFF;
    circular_buffer_push_back_uint8(&extraInputBuffer, isr_tmp_byte);
  }
}
