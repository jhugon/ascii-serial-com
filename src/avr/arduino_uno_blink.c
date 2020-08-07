#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

int main(void) {

  DDRB = 0x1;

  while (1) {
    PORTB = 0x1;
    _delay_ms(1000);
    PORTB = 0x0;
    _delay_ms(1000);
  }
  return 0;
}
