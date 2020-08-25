#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

// F5 is the yellow LED
// F6 is the user switch

int main(void) {

  PORTF.OUTSET = _BV(5);

  while (1) {
    PORTF.OUTTGL = _BV(5);
    _delay_ms(1000);
  }
  return 0;
}
