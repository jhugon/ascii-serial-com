#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "avr/avr_timers.h"

#define F_CPU 16000000UL
// period = 1024*256*n_overflows / F_CPU
#define n_overflows F_CPU / 1024 / 256 / 2

int main(void) {

  TIMER0_Init(5, 0, 1);

  DDRB |= _BV(5);

  sei();

  while (1) {
  }
  return 0;
}

ISR(TIMER0_OVF_vect) {
  static uint16_t timer_overflow_count;
  timer_overflow_count += 1;
  if (timer_overflow_count == n_overflows) {
    PORTB ^= _BV(5); // flip bit
    timer_overflow_count = 0;
  }
}
