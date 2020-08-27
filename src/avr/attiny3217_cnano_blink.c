#include <avr/io.h>
#define F_CPU 3333333UL // 20 MHz / 6
#include <util/delay.h>

// A3 is the yellow user LED
// B7 is the user button

int main(void) {

  PORTA.DIRSET = _BV(3);

  while (1) {
    PORTA.OUTTGL = _BV(3);
    _delay_ms(1000);
  }
  return 0;
}
