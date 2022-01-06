/** \file
 *
 * \brief Outputs a pulser and (one day will) measures pulses
 *
 * Outputs configurable pulses on the LED pin
 *
 * Register map is documented at \ref register_map
 *
 */

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

#include "arm/stm_timers.h"
#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "circular_buffer.h"
#include "millisec_timer.h"

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOA
#define TIM_LED TIM2
#define TIM_OC_LED TIM_OC1 // also AF2
#define RCC_TIM_LED RCC_TIM2

// pulser stuff
#define pulser_prescale 16000
#define pulser_period 4000
#define pulser_width 500

/////////////////////////////////

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;
uint16_t nExceptions;

MILLISEC_TIMER_SYSTICK_IT;

/////////////////////////////////

uint32_t optionFlags = 0;

#define nRegs 7

/** \brief Register Map
 *
 * ## Register Map
 *
 * |Register Number | Description | r/w |
 * | -------------- |------------ | --- |
 * | 0 | PORTA input data register, bit 5 is LED | r |
 * | 1 | PORTA output data register, bit 5 is LED | r, bit 5 is w |
 * | 2 | optionFlags: see below | r/w |
 * | 3 | Current millisecond_timer value | r |
 * | 4 | LED pulser prescaler | r/w 16 bits |
 * | 5 | LED pulser period | r/w 16 bits |
 * | 6 | LED pulser pulse length | r/w 16 bits |
 *
 * ### Option flags
 *
 * |Big Number | Description |
 * | -------------- |------------ |
 *
 * @see register_write_masks
 *
 */
volatile REGTYPE *register_map[nRegs] = {
    &GPIOA_IDR, // input data reg
    &GPIOA_ODR, // output data reg
    &optionFlags,
    &MILLISEC_TIMER_NOW, // millisec timer value
    &TIM_PSC(TIM_LED),   // LED pulser prescaler
    &TIM_CCR1(TIM_LED),  // LED pulser period
    &TIM_CCR2(TIM_LED),  // LEd pulser pulse length
};

/** \brief Write masks for \ref register_map
 *
 * These define whether the given register in register_map is writable or not
 *
 */
REGTYPE register_write_masks[nRegs] = {
    0,          // input data reg
    1 << 5,     // output data reg
    0xFFFFFFFF, // option flags
    0,          // MILLISEC_TIMER_NOW
    0xFFFF,     // LED pulser prescaler
    0xFFFF,     // LED pulser period
    0xFFFF,     // LEd pulser pulse length
};

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
circular_buffer_uint8 *asc_in_buf;
circular_buffer_uint8 *asc_out_buf;

///////////////////////////////////

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

  Try {
    ascii_serial_com_register_pointers_init(&reg_pointers_state, register_map,
                                            register_write_masks, nRegs);
    ascii_serial_com_device_init(&ascd, &ascd_config);
    asc_in_buf = ascii_serial_com_device_get_input_buffer(&ascd);
    asc_out_buf = ascii_serial_com_device_get_output_buffer(&ascd);

    circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                               extraInputBuffer_raw);

    millisec_timer_systick_setup(rcc_ahb_frequency);
    usart_setup();

    rcc_periph_clock_enable(RCC_GPIO_LED);
    rcc_periph_clock_enable(RCC_TIM_LED);
    setup_timer_periodic_output_pulse(TIM_LED, pulser_prescale, pulser_period,
                                      pulser_width, TIM_OC_LED, PORT_LED,
                                      PIN_LED);
    timer_enable_counter(TIM_LED);
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

      // parse and handle received messages
      ascii_serial_com_device_receive(&ascd);

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
