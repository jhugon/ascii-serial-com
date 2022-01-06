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

/** \brief Initialize a timer to capture periodic pulses on an input pin
 *
 * This really just sets up a timer for PWM input capture
 *
 * ## Usage
 *
 * The pulse period should be available in TIM_CCR1(<timer>) in timer ticks
 *
 * The pulse length should be available in TIM_CCR2(<timer>) in timer ticks
 *
 * So duty cycle is TIM_CCR2(<timer>) / TIM_CCR1(<timer>)
 *
 * Polling: `bool flag = timer_get_flag(<timer>,TIM_SR_CC1IF);`, if true read
 * out `TIM_CCR2(<timer>)` and `TIM_CCR1(<timer>)`, then check the overcapture
 * bits: `timer_get_flag(<timer>,TIM_SR_CC1OF) &&
 * timer_get_flag(<timer>,TIM_SR_CC2OF)`.
 *
 * Interrupt: Setup the interrupt on CC1:
 * `timer_enable_irq(<timer>,TIM_DIER_CC1IE)`, then in the handler read out
 * both CCRs and then check the overcapture bits:
 * `timer_get_flag(<timer>,TIM_SR_CC1OF) &&
 * timer_get_flag(<timer>,TIM_SR_CC2OF)`
 *
 * DMA: ?
 *
 * If overcapture happens, you have to clear the flag with
 * `timer_get_flag(<timer>,TIM_SR_CC1OF)` or
 * `timer_get_flag(<timer>,TIM_SR_CC2OF)`.
 *
 * ## Notes
 *
 * **The user must setup the peripheral clocks for both this timer and the
 * input port.**
 *
 * **The user must enable the counter when ready with:**
 * `timer_enable_counter(<timer>);`
 *
 * **Uses input compare channels 1 and 2, so timer must have 2 IC units!**
 *
 * Sets timer in: upcount, edge-aligned, not one-pulse mode, using the
 * un-divided peripheral clock as input to the prescaler. This should be
 * available on all general-purpose timers.
 *
 * ## Registers Relevant to Ascii-Serial-Com
 *
 * `TIM_PSC(<timer>)`: which should be set to the prescaler value in peripheral
 * clock ticks
 *
 * `TIM_CCR1(<timer>)`: read the pulse period in timer ticks
 *
 * `TIM_CCR2(<timer>)`: read the pulse length in timer ticks
 *
 * ## Parameters
 *
 * timer: the timer you want to use like TIM2
 *
 * prescale: the number of peripher clock ticks between timer ticks, uint32_t
 * (but only ever 16 bit)
 *
 * max_timer_counts: you probably want 0xFFFF for 16 bit counters and
 * 0xFFFFFFFF for 32 bit ones.
 *
 * input: which input compare input: TI1,TI2,TI3 or TI4 (don't quote; some not
 * avaiable on all timers)
 *
 */
#define setup_timer_capture_pwm_input(timer, prescale, max_timer_counts,       \
                                      input)                                   \
  timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP); \
  timer_ic_set_input(timer, TIM_IC1, TIM_IC_IN_##input);                       \
  timer_ic_set_polarity(timer, TIM_IC1, TIM_IC_RISING);                        \
  timer_ic_set_input(timer, TIM_IC2, TIM_IC_IN_##input);                       \
  timer_ic_set_polarity(timer, TIM_IC2, TIM_IC_FALLING);                       \
  timer_slave_set_trigger(timer, TIM_SMCR_TS_TI1FP1);                          \
  timer_slave_set_mode(timer, TIM_SMCR_SMS_RM);                                \
  timer_ic_enable(timer, TIM_IC1);                                             \
  timer_ic_enable(timer, TIM_IC2);                                             \
  TIM_PSC(timer) = prescale;                                                   \
  TIM_ARR(timer) = max_timer_counts;

#endif
