#include "millisec_timer.h"
#ifdef linux
#include <time.h>
#endif

/** \file */

void millisec_timer_set_rel(millisec_timer *timer,
                            const millisec_timer_unit_t now,
                            const millisec_timer_unit_t rel) {
  timer->enabled = true;
  timer->set_time = now;
  timer->expire_time = now + rel;
}

bool millisec_timer_is_expired(millisec_timer *timer,
                               const millisec_timer_unit_t now) {
  if (timer->enabled) {
    if ((timer->expire_time == timer->set_time) ||
        (timer->expire_time > timer->set_time &&
         (now >= timer->expire_time || now < timer->set_time)) ||
        (timer->expire_time < timer->set_time && now >= timer->expire_time &&
         now < timer->set_time)) {
      timer->enabled = false;
      return true;
    }
  }
  return false;
}
bool millisec_timer_is_expired_repeat(millisec_timer *timer,
                                      const millisec_timer_unit_t now) {
  if (timer->enabled) {
    if (timer->expire_time == timer->set_time) {
      return true;
    } else if ((timer->expire_time > timer->set_time &&
                (now >= timer->expire_time || now < timer->set_time)) ||
               (timer->expire_time < timer->set_time &&
                now >= timer->expire_time && now < timer->set_time)) {
      const millisec_timer_unit_t old_expire_time = timer->expire_time;
      timer->expire_time =
          old_expire_time + (old_expire_time - timer->set_time);
      timer->set_time = old_expire_time;
      return true;
    }
  }
  return false;
}

#ifdef __ARM_ARCH
void millisec_timer_systick_setup(uint32_t ahb_frequency) {
  systick_set_clocksource(1 << STK_CSR_CLKSOURCE_LSB); // always AHB clock
  STK_CVR = 0;                                         // clear clock
  systick_set_reload(ahb_frequency / 1000);
  systick_counter_enable();
  systick_interrupt_enable();
}
#endif
