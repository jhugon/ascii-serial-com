#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "arm/stm_usart.h"
#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "circular_buffer.h"

// USART2 should be connected through USB
#define ASC_USART USART2

char dataBuffer[MAXDATALEN];
ascii_serial_com asc;
circular_buffer_uint8 *asc_in_buf;
circular_buffer_uint8 *asc_out_buf;

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;
char ascVersion, appVersion, command;
size_t dataLen;

uint16_t nExceptions;

uint8_t tmp_byte = 0;

int main(void) {

  Try {
    ascii_serial_com_init(&asc);
    ascii_serial_com_set_ignore_CRC_mismatch(&asc);
    asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
    asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

    circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                               extraInputBuffer_raw);

    // USART2 TX: PA2 AF1
    // USART2 RX: PA3 AF1
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    setup_usart(ASC_USART, 9600, GPIOA, GPIO2, GPIO_AF1, GPIOA, GPIO3,
                GPIO_AF1);
    nvic_enable_irq(NVIC_USART2_IRQ);
    usart_enable_rx_interrupt(ASC_USART);
    usart_enable(ASC_USART);
  }
  Catch(e) { return e; }

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
          (USART_ISR(ASC_USART) & USART_ISR_TXE)) {
        tmp_byte = circular_buffer_pop_front_uint8(asc_out_buf);
        usart_send(ASC_USART, tmp_byte);
      }
    }
    Catch(e) { nExceptions++; }
  }

  return 0;
}

def_usart_isr_push_rx_to_circ_buf(usart2_isr, ASC_USART, &extraInputBuffer)
