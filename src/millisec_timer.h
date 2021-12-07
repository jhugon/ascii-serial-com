#ifndef MILLISEC_TIMER_H
#define MILLISEC_TIMER_H

/** \file Portable millisecond timer */

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef linux
#include <stdio.h>
#endif
#ifdef __ARM_ARCH
#include <libopencm3/cm3/systick.h>
#endif

typedef uint32_t millisec_timer_unit_t;

/** \brief portable millisecond timer
 *
 *  It's designed to be easy to use and handle wraparound properly (which
 * happens every 49.7 days for uint32_t).
 *
 */
typedef struct __millisec_timer {
  bool enabled;
  millisec_timer_unit_t set_time;
  millisec_timer_unit_t expire_time;
} millisec_timer;

/** \brief Set timer to expire in the future
 *
 * Sets and enables timer to expire rel ms in the future.
 *
 * This sets the set_time to now and the expire_time to set_time + rel
 *
 */
void millisec_timer_set_rel(millisec_timer *timer,
                            const millisec_timer_unit_t now,
                            const millisec_timer_unit_t rel);

/** \brief Check if timer has expired & if so, disable it
 *
 * If timer is enabled and it's expired, return true and disable the timer.
 * If not enabled or the time hasn't expired, return false.
 *
 */
bool millisec_timer_is_expired(millisec_timer *timer,
                               const millisec_timer_unit_t now);

/** \brief Check if timer has expired & if so, re-enable for the same interval
 *
 * If timer is enabled and it's set time has expired:
 *  1) set the new expiration time to be the old expire time + (old expire time
 * - old set time) 2) set the new set time to be the old expire time 3) return
 * true. If not enabled or the time hasn't expired, return false.
 *
 */
bool millisec_timer_is_expired_repeat(millisec_timer *timer,
                                      const millisec_timer_unit_t now);

#ifdef __ARM_ARCH

/** \brief A counter that increments every millisecond
 *
 * This value will increment every millisecond if you put:
 *
 * 1) #include <libopencm3/cm3/nvic.h> at the top of your main file
 *
 * 2) MILLISEC_TIMER_SYSTICK_IT; outside of any function to
 *    define the systick interrupt
 *
 * 3) millisec_timer_systick_setup(rcc_ahb_frequency); in the setup
 *    portion of your main function. It's probably best to put
 *    before other things.
 *
 */
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic push
static uint32_t MILLISEC_TIMER_NOW = 0;
#pragma GCC diagnostic pop

/** \brief Millisecond timer SysTick interrupt
 *
 *  Implements an interrupt handler that increments MILLISEC_TIMER_NOW
 *
 */
#define MILLISEC_TIMER_SYSTICK_IT                                              \
  void sys_tick_handler(void) { MILLISEC_TIMER_NOW++; }

/** \brief Setup the systick timer
 *
 * Configures the systick timer to raise the interrupt every millisecond.
 *
 */
void millisec_timer_systick_setup(uint32_t ahb_frequency);
#endif

#endif
