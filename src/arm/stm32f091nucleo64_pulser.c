/** \file
 *
 * \brief Outputs a pulser and (one day will) measures pulses
 *
 * Outputs configurable pulses on the LED pin, PA5, which is "D13" on the
 * Arduino connector
 *
 * Reads input pulses on PA8, which is "D7" on the Arduino connector
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
#include "arm/stm_usart.h"
#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "circular_buffer.h"
#include "millisec_timer.h"

// USART2 should be connected through USB
#define ASC_USART USART2

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOA
#define TIM_LED TIM2
#define TIM_OC_LED TIM_OC1
#define RCC_TIM_LED RCC_TIM2
#define AF_TIM_LED GPIO_AF2

// pulser stuff
#define pulser_tick_freq 1000 // should do 1 tick per ms
#define pulser_period 1000
#define pulser_width 500

// Input compare stuff on PA8 AF2, TIM1 input 1 (Arduino connector pin D7)
#define IC_PORT GPIOA
#define IC_PIN GPIO8
#define IC_RCC_GPIO RCC_GPIOA
#define IC_TIM TIM1
#define IC_RCC_TIM RCC_TIM1
#define IC_TI TIM_IC_IN_TI1
#define IC_AF GPIO_AF2
uint32_t ic_pulse_length;
uint32_t ic_period;
uint32_t ic_overrun;

/////////////////////////////////

CEXCEPTION_T e;
uint16_t nExceptions;

MILLISEC_TIMER_SYSTICK_IT;

/////////////////////////////////

uint32_t optionFlags = 0;

#define nRegs 10

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
 * | 7 | Input capture pulse period | r |
 * | 8 | Input capture pulse width | r |
 * | 9 | Input capture overrun flags (bit 0 for period and 4 for width) | r |
 *
 * @see register_write_masks
 *
 */
volatile REGTYPE *register_map[nRegs] = {
    &GPIOA_IDR,          // input data reg
    &GPIOA_ODR,          // output data reg
    &optionFlags,        // option flags
    &MILLISEC_TIMER_NOW, // millisec timer value
    &TIM_PSC(TIM_LED),   // LED pulser prescaler
    &TIM_ARR(TIM_LED),   // LED pulser period
    &TIM_CCR1(TIM_LED),  // LED pulser pulse length
    &ic_period,          // Input capture pulse period
    &ic_pulse_length,    // Input capture pulse length
    &ic_overrun,         // Input capture overrun flags
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
    0,          // Input capture pulse period
    0,          // Input capture pulse length
    0,          // Input capture overrun flags
};

DECLARE_ASC_DEVICE_W_REGISTER_POINTERS()

///////////////////////////////////

uint8_t tmp_byte = 0;
int main(void) {

  Try {
    SETUP_ASC_DEVICE_W_REGISTER_POINTERS(register_map, register_write_masks,
                                         nRegs);

    millisec_timer_systick_setup(rcc_ahb_frequency);

    // USART2 TX: PA2 AF1
    // USART2 RX: PA3 AF1
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    setup_usart(ASC_USART, 9600, GPIOA, GPIO2, GPIO_AF1, GPIOA, GPIO3,
                GPIO_AF1);
    nvic_enable_irq(NVIC_USART2_IRQ);
    usart_enable_rx_interrupt(ASC_USART);
    usart_enable(ASC_USART);

    rcc_periph_clock_enable(RCC_GPIO_LED);
    rcc_periph_clock_enable(RCC_TIM_LED);
    setup_timer_periodic_output_pulse(
        TIM_LED, rcc_get_timer_clk_freq(TIM_LED) / pulser_tick_freq,
        pulser_period, pulser_width, TIM_OC_LED, PORT_LED, PIN_LED, AF_TIM_LED);
    timer_enable_counter(TIM_LED);

    rcc_periph_clock_enable(IC_RCC_GPIO);
    rcc_periph_clock_enable(IC_RCC_TIM);
    setup_timer_capture_pwm_input(
        IC_TIM, rcc_get_timer_clk_freq(IC_TIM) / pulser_tick_freq, 0xFFFF,
        IC_TI, IC_PORT, IC_PIN, IC_AF);
    timer_enable_counter(IC_TIM);
  }
  Catch(e) { return e; }

  while (1) {
    Try {
      HANDLE_ASC_COMM_IN_POLLING_LOOP(ASC_USART);

      if (timer_get_flag(IC_TIM, TIM_SR_CC1IF)) {
        ic_pulse_length = TIM_CCR2(IC_TIM);
        ic_period = TIM_CCR1(IC_TIM);
        ic_overrun = 0;
        if (timer_get_flag(IC_TIM, TIM_SR_CC1OF)) {
          ic_overrun |= 1;
          timer_clear_flag(IC_TIM, TIM_SR_CC1OF);
        }
        if (timer_get_flag(IC_TIM, TIM_SR_CC2OF)) {
          ic_overrun |= 1 << 4;
          timer_clear_flag(IC_TIM, TIM_SR_CC2OF);
        }
      }
    }
    Catch(e) { nExceptions++; }
  }

  return 0;
}

def_usart_isr_push_rx_to_circ_buf(usart2_isr, ASC_USART, &extraInputBuffer)
