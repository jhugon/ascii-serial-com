#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOC

static void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIO_LED);

  gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

int main(void) {
  int i;

  gpio_setup();

  while (1) {
    gpio_toggle(PORT_LED, PIN_LED);
    for (i = 0; i < 1000000; i++) {
      __asm__("nop");
    }
  }

  return 0;
}
