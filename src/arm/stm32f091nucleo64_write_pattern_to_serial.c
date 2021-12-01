#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOA

// USART2 should be connected through USB
// USART2 TX: PA2 AF1
// USART2 RX: PA3 AF1
static void usart_setup(void) {
  rcc_periph_clock_enable(RCC_USART2);
  rcc_periph_clock_enable(RCC_GPIOA);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);

  gpio_set_af(GPIOA, GPIO_AF1, GPIO2);
  gpio_set_af(GPIOA, GPIO_AF1, GPIO3);

  usart_set_baudrate(USART2, 9600);
  usart_set_databits(USART2, 8);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_stopbits(USART2, USART_CR2_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  usart_enable(USART2);
}

static void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIO_LED);

  gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

uint8_t counter = 0;

int main(void) {
  int i;

  gpio_setup();
  usart_setup();

  while (1) {
    usart_send_blocking(USART2, '0' + counter);
    if (counter >= 9) {
      counter = 0;
      gpio_toggle(PORT_LED, PIN_LED);
    } else {
      counter++;
    }
    for (i = 0; i < 10000; i++) {
      __asm__("nop");
    }
  }

  return 0;
}
