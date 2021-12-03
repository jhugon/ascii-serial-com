/*
 * Register map:
 *
 * 0: PORTA input data register, bit 5 is LED (r)
 * 1: PORTA output data register, bit 5 is LED (r, bit 5 is writable)
 *
 */

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "circular_buffer.h"

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOA

/////////////////////////////////

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;
uint16_t nExceptions;

/////////////////////////////////

#define nRegs 2
volatile REGTYPE *regPtrs[nRegs] = {
    &GPIOA_IDR, // input data reg
    &GPIOA_ODR, // output data reg
};

REGTYPE masks[nRegs] = {0, 1 << 5};

typedef struct stream_state_struct {
  uint8_t on;
} on_off_stream_state;
void handle_nf_messages(ascii_serial_com *asc, char ascVersion, char appVersion,
                        char command, char *data, size_t dataLen,
                        void *state_vp);

uint32_t counter = 0;
on_off_stream_state stream_state;

ascii_serial_com_device ascd;
ascii_serial_com_register_pointers reg_pointers_state;
ascii_serial_com_device_config ascd_config = {
    .func_rw = ascii_serial_com_register_pointers_handle_message,
    .state_rw = &reg_pointers_state,
    .func_nf = handle_nf_messages,
    .state_nf = &stream_state};

///////////////////////////////////

static void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIO_LED);

  gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

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

  ascii_serial_com_register_pointers_init(&reg_pointers_state, regPtrs, masks,
                                          nRegs);
  ascii_serial_com_device_init(&ascd, &ascd_config);
  circular_buffer_uint8 *asc_in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  circular_buffer_uint8 *asc_out_buf =
      ascii_serial_com_device_get_output_buffer(&ascd);

  circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                             extraInputBuffer_raw);

  gpio_setup();
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

      // parse and handle received messages
      ascii_serial_com_device_receive(&ascd);

      if (stream_state.on && circular_buffer_get_size_uint8(asc_out_buf) == 0) {
        char counter_buffer[8];
        convert_uint32_to_hex(counter, counter_buffer, true);
        ascii_serial_com_device_put_s_message_in_output_buffer(
            &ascd, '0', '0', counter_buffer, 8);
        counter++;
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

void handle_nf_messages(__attribute__((unused)) ascii_serial_com *asc,
                        __attribute__((unused)) char ascVersion,
                        __attribute__((unused)) char appVersion, char command,
                        __attribute__((unused)) char *data,
                        __attribute__((unused)) size_t dataLen,
                        void *state_vp) {
  on_off_stream_state *state = (on_off_stream_state *)state_vp;
  if (command == 'n') {
    state->on = 1;
  } else if (command == 'f') {
    state->on = 0;
  }
}
