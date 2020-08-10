#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

int main(void) {

  DDRB |= _BV(7);

  while (1) {
    PORTB |= _BV(7);
    _delay_ms(1000);
    PORTB &= ~(_BV(7));
    _delay_ms(1000);
  }
  return 0;
}
