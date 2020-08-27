#include <avr/io.h>
#define F_CPU 3333333UL // 20 MHz / 6
#include <util/delay.h>

// C0 is the user LED
// C5 is the user button
// A6, A7 are the capacitive touch buttons

int main(void) {

  PORTC.DIRSET = _BV(0);

  while (1) {
    PORTC.OUTTGL = _BV(0);
    _delay_ms(1000);
  }
  return 0;
}
