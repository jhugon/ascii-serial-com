#ifndef stm_TIMERS_H
#define stm_TIMERS_H

/** \file
 *
 * \brief To be used with the timer peripherals on STM32 microcontrollers
 *
 * */

#include <libopencm3/stm32/timer.h>
#include <stdint.h>

/** \brief Initialize a timer to output a periodic pulse on an output pin
 *
 * This really just sets up a timer in edge-aligned PWM mode.
 *
 * ## Notes
 *
 * **The user must setup the peripheral clocks for both this timer and the
 * output port.**
 *
 * **The user must enable the counter when ready with:**
 * `timer_enable_counter(<timer>);`
 *
 * **This macro takes care of setting up the GPIO pin mode.**
 *
 * Uses output compare unit 1 on the timer. Make sure this is available on the
 * timer.
 *
 * Sets timer in: upcount, edge-aligned, not one-pulse mode, using the
 * un-divided peripheral clock as input to the prescaler. This should be
 * available on all general-purpose timers.
 *
 * ## Registers Relevant to Ascii-Serial-Com
 *
 * `TIM_ARR(<timer>)`: which should be set to period in timer ticks
 *
 * `TIM_CCR1(<timer>)`: which should be set to pulse_length in timer ticks
 *
 * `TIM_PSC(<timer>)`: which should be set to the prescaler value in peripheral
 * clock ticks
 *
 * ## Parameters
 *
 * timer: the timer you want to use like TIM2
 *
 * prescale: the number of peripher clock ticks between timer ticks, uint32_t
 * (but only ever 16 bit)
 *
 * period: The number of timer ticks between pulse rising edges, uint32_t
 *
 * pulse_length: length of pulse in timer ticks, uint32_t
 *
 * gpio_port: GPIOA, GPIOB, ...
 *
 * gpio_pin: GPIO0, GPIO1, ...
 *
 */
#define setup_timer_periodic_output_pulse(timer, prescale, period,             \
                                          pulse_length, gpio_port, gpio_pin)   \
  gpio_set_output_options(gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,          \
                          gpio_pin);                                           \
  timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP); \
  timer_set_oc_mode(timer, TIM_OC1, TIM_OCM_PWM1);                             \
  timer_enable_oc_output(timer, TIM_OC1);                                      \
  TIM_PSC(timer) = prescale;                                                   \
  TIM_ARR(timer) = period;                                                     \
  TIM_CCR1(timer) = pulse_length;

#endif
