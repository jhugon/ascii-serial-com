#include "arm/stm_usart.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOA

// USART2 should be connected through USB
#define ASC_USART USART2

static void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIO_LED);

  gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

uint8_t counter = 0;

int main(void) {

  gpio_setup();

  // USART2 TX: PA2 AF1
  // USART2 RX: PA3 AF1
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART2);
  setup_usart(ASC_USART, 9600, GPIOA, GPIO2, GPIO_AF1, GPIOA, GPIO3, GPIO_AF1);
  // usart_set_mode(ASC_USART,USART_MODE_TX); // this was here before, is it
  // still needed?
  usart_enable(ASC_USART);

  while (1) {
    usart_send_blocking(ASC_USART, '0' + counter);
    if (counter >= 9) {
      counter = 0;
      //      gpio_toggle(PORT_LED, PIN_LED);
    } else {
      counter++;
    }
    //   for (int i = 0; i < 10000; i++) {
    //     __asm__("nop");
    //   }
  }

  return 0;
}
