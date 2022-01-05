#ifndef stm_TIMERS_H
#define stm_TIMERS_H

/** \file */

#include <libopencm3/stm32/timer.h>
#include <stdint.h>

/** \brief Initialize a timer to output a periodic pulse on an output pin
 *
 * This really just sets up a timer in simple upcounter mode with output compare
 *
 * **The user must setup the peripheral clocks for both this timer and the
 * output port.**
 *
 * **The user must enable the counter when ready with:**
 * `timer_enable_counter(<TIMER>);`
 *
 * **This macro takes care of setting up the pin mode.**
 *
 * Uses output compare unit 1 on the timer.
 *
 * Parameters:
 *
 * timer: the timer you want to use like TIM2
 *
 * clock_div: One of:
 *
 *      TIM_CR1_CKD_CK_INT
 *      TIM_CR1_CKD_CK_INT_MUL_2
 *      TIM_CR1_CKD_CK_INT_MUL_4
 *      (where MUL really means divide the clock)
 *
 * period: The number of clock ticks between pulse rising edges, uint32_t
 *
 * pulse_length: length of pulse in clock ticks, uint32_t
 *
 * gpio_port: GPIOA, GPIOB, ...
 *
 * gpio_pin: GPIO0, GPIO1, ...
 *
 */
#define setup_timer_periodic_output_pulse(timer, clock_div, period,            \
                                          pulse_length, gpio_port, gpio_pin)   \
  gpio_set_output_options(gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,          \
                          gpio_pin);                                           \
  timer_set_mode(timer, clock_div, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);          \
  timer_set_period(timer, period);                                             \
  timer_set_oc_mode(timer, TIM_OC1, TIM_OCM_PWM2);                             \
  timer_enable_oc_output(timer, TIM_OC1);                                      \
  timer_enable_oc_value(timer, TIM_OC1, period - pulse_length);

#endif
