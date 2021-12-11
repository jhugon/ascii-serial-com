/** \file
 *
 * \brief Streams ADC data from pin A0 (happens to be labelled A0 on the Arduino
 * connector of nucleo-f091rc board)
 *
 * The time between ADC conversions is set in register 6, in milliseconds.
 *
 * Through changing option flag, can instead stream a 32 bit counter
 *
 * DAC channel 1 (A4 and Arduino A2 on board) is also enabled
 * and may be changed by writing to register 3. Period between
 * DAC conversions is configurable as is trinagle/noise generation.
 *
 * A register write can also turn on/off the LED
 *
 * Register map is documented at \ref register_map
 *
 */

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "circular_buffer.h"
#include "millisec_timer.h"

#define PORT_LED GPIOA
#define PIN_LED GPIO5
#define RCC_GPIO_LED RCC_GPIOA

/////////////////////////////////

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;
uint16_t nExceptions;

MILLISEC_TIMER_SYSTICK_IT;
millisec_timer adc_timer;
uint32_t adc_sample_period_ms = 1000;
uint32_t adc_n_overruns = 0;
millisec_timer dac_reset_timer;
#define DAC_RESET_TIME_MS 500

/////////////////////////////////

uint32_t optionFlags = 0;

#define nRegs 10

/** \brief Register Map
 *
 * ## Register Map
 *
 * |Register Number | Description | r/w |
 * | -------------- |------------ | --- |
 * | 0 | PORTA input data register, bit 5 is LED | r |
 * | 1 | PORTA output data register, bit 5 is LED | r, bit 5 is w |
 * | 2 | optionFlags: see below | r/w |
 * | 3 | DAC Channel 1 output val | r, bottom 12 bits w |
 * | 4 | Reserved for DAC Channel 2, but not implemented | r |
 * | 5 | Current millisecond_timer value | r |
 * | 6 | Period between ADC samples in ms * | r/w |
 * | 7 | Number of times ADC overrun flag has been set | r |
 * | 8 | Time between DAC conversion in 100 us * | r/w 16 bits |
 * | 9 | DAC waveform & waveform amplitude sel. * | r, w bits 6-10 |
 *
 * ### Option flags
 *
 * |Big Number | Description |
 * | -------------- |------------ |
 * | 0 | if 0: stream ADC, if 1: stream counter |
 * | 1 | if 1: disable & re-enable DAC, then reset bit to 0, if 0: nothing |
 *
 * Register 6: ADC sample period only written to ADC register when 'n' command
 * received
 *
 * Register 8: default 100, so update every 10 ms
 *
 * ### Waveform generation with register 9
 *
 * Register 9: default 0. Bits 6-7 (start from 0) control waveform generation.
 * 00: disabled, 01: noise, 1x: triangle. Bits 8-11 are amplitude: where the
 * max-min is 2^(n+1)-1, where n is the value (at least for triangle), maxing
 * out at 0b1011=0xB. The triangle starts from reg 3 and goes up to the max-min
 * value programmed, then comes back down. The noise is centered around
 * register 3.
 *
 * So, write to the register: 0xYZ0, where Y is the amplitude: 0-B, and Z is
 * the waveform type: 0 for none, 1 for nosie, 8 for triangle.
 *
 * I'd say the noise has an RMS in the region of 10 counts. The mask doesn't
 * change the RMS that much.
 *
 * **It may be necessary to reset the DAC (see option flags) to change the
 * amplitude and/or disable waveform generation**
 *
 * Register 9 examples:
 *
 * - triangle with max-min = 127: 0b10110000000 = 0x680
 * - full scale triangle: write 9 0xB80, write 3 0, write 8 3
 * - noise with large amplitude: 0xB10
 * - flat, waveform disabled: 0
 *
 * @see register_write_masks
 *
 */
volatile REGTYPE *register_map[nRegs] = {
    &GPIOA_IDR, // input data reg
    &GPIOA_ODR, // output data reg
    &optionFlags,
    &DAC_DHR12R1(DAC1),    // DAC Channel 1 Data holding register
    &DAC_DHR12R2(DAC1),    // DAC Channel 2 Data holding register (not active)
    &MILLISEC_TIMER_NOW,   // millisec timer value
    &adc_sample_period_ms, // Time between ADC samples in ms
    &adc_n_overruns,       // number of times overrun bit has been set
    &TIM_ARR(TIM6),        // time between DAC conversions in 100 us
    &DAC_CR(DAC1), // waveform and waveform amplitude selection only want bits
                   // 6-11 (start from 0)
};

/** \brief Write masks for \ref register_map
 *
 * These define whether the given register in register_map is writable or not
 *
 */
REGTYPE register_write_masks[nRegs] = {
    0,          // input data reg
    1 << 5,     // output data reg
    0xFFFFFFFF, // option flags
    0xFFF,      // DAC_DHR12R1
    0,          // DAC_DHR12R2
    0,          // MILLISEC_TIMER_NOW
    0xFFFFFFFF, // adc_sample_period_ms
    0,          // adc_n_overruns
    0xFFFF,     // TIM_ARR(TIM6)
    0xFC0,      // DAC_CR(DAC), w bits 6-11
};

typedef struct stream_state_struct {
  uint8_t on;
} on_off_stream_state;
void handle_nf_messages(ascii_serial_com *asc, char ascVersion, char appVersion,
                        char command, char *data, size_t dataLen,
                        void *state_vp);

uint32_t counter = 0;
on_off_stream_state stream_state;

ascii_serial_com_device ascd;
ascii_serial_com_register_pointers reg_pointers_state;
ascii_serial_com_device_config ascd_config = {
    .func_rw = ascii_serial_com_register_pointers_handle_message,
    .state_rw = &reg_pointers_state,
    .func_nf = handle_nf_messages,
    .state_nf = &stream_state};
circular_buffer_uint8 *asc_in_buf;
circular_buffer_uint8 *asc_out_buf;

///////////////////////////////////

static void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIO_LED);

  gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

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

  nvic_enable_irq(NVIC_USART2_IRQ);

  usart_set_baudrate(USART2, 9600);
  usart_set_databits(USART2, 8);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_stopbits(USART2, USART_CR2_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX_RX);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  usart_enable_rx_interrupt(USART2);

  usart_enable(USART2);
}

static void adc_setup(void) {
  // With nChans == 1 and ADC_MODE_SCAN, there is only one conversion per
  // "trigger" You can manually trigger by setting ADSTART in ADC_CR
  //      ADSTART will stay set and only be cleared when EOSEQ is set
  // After the conversion is complete EOC and EOSEQ are both set (in ADC_ISR)
  // You can read the data from ADC_DR
  rcc_periph_clock_enable(RCC_ADC);
  rcc_periph_clock_enable(RCC_GPIOA);

  // Only pins A0=ADC_IN0 happens to also be Arduino A0
  gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
#define nChans 1
  uint8_t adc_channel_sequence[nChans] = {0};

  adc_power_off(ADC);
  adc_set_clk_source(ADC, ADC_CLKSOURCE_ADC);
  adc_calibrate(ADC);
  adc_set_operation_mode(
      ADC, ADC_MODE_SCAN); // ADC_MODE_SCAN is ~cont and ~discon;
                           // ADC_MODE_SCAN_INFINITE is cont and ~dicon
  adc_disable_external_trigger_regular(ADC);
  adc_set_right_aligned(ADC);
  adc_enable_temperature_sensor();
  adc_set_sample_time_on_all_channels(ADC, ADC_SMPTIME_071DOT5);
  adc_set_regular_sequence(ADC, nChans, adc_channel_sequence);
  adc_set_resolution(ADC, ADC_RESOLUTION_12BIT);
  adc_disable_analog_watchdog(ADC);
  adc_power_on(ADC); // this sync waits until ADRDY is set
}

static void dac_setup(void) {
  // Setup TIM6
  rcc_periph_clock_enable(RCC_TIM6);
  TIM_PSC(TIM6) = rcc_ahb_frequency / 100 - 1; // tick every 100 us
  TIM_ARR(TIM6) = 100;                         // by default, update every 10 ms
  TIM_CR2(TIM6) |= TIM_CR2_MMS_UPDATE;         // trigger output on update
  TIM_CR1(TIM6) |= TIM_CR1_ARPE; // wait for update to update the value of
                                 // auto-reload shadow reg
  TIM_CR1(TIM6) |= TIM_CR1_CEN;  // enable

  // Setting this up to trigger using timer 6, so setting that up here too.
  rcc_periph_clock_enable(RCC_DAC);
  rcc_periph_clock_enable(RCC_GPIOA);

  // Pin A4 is DAC1 output and Arduino A2 on board
  // Pin A5 is DAC2 output and Arduino D13 on board
  // Only enable DAC1 for now
  gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);

  dac_disable(DAC1, DAC_CHANNEL1);
  dac_buffer_enable(DAC1, DAC_CHANNEL1);
  dac_disable_waveform_generation(DAC1, DAC_CHANNEL1);
  dac_set_trigger_source(DAC1, DAC_CR_TSEL1_T6);
  dac_trigger_enable(DAC1, DAC_CHANNEL1);
  dac_enable(DAC1, DAC_CHANNEL1);
}

uint8_t tmp_byte = 0;
int main(void) {

  Try {
    ascii_serial_com_register_pointers_init(&reg_pointers_state, register_map,
                                            register_write_masks, nRegs);
    ascii_serial_com_device_init(&ascd, &ascd_config);
    asc_in_buf = ascii_serial_com_device_get_input_buffer(&ascd);
    asc_out_buf = ascii_serial_com_device_get_output_buffer(&ascd);

    circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                               extraInputBuffer_raw);

    millisec_timer_systick_setup(rcc_ahb_frequency);
    gpio_setup();
    adc_setup();
    dac_setup();
    usart_setup();
  }
  Catch(e) { return e; }

  while (1) {
    Try {
      // Move data from extraInputBuffer to asc_in_buf
      if (!circular_buffer_is_empty_uint8(&extraInputBuffer)) {
        CM_ATOMIC_BLOCK() {
          tmp_byte = circular_buffer_pop_front_uint8(&extraInputBuffer);
        }
        circular_buffer_push_back_uint8(asc_in_buf, tmp_byte);
      }

      // parse and handle received messages
      ascii_serial_com_device_receive(&ascd);

      if ((optionFlags >> 1) &
          1) { // reset DAC, needed b/c it's hard to disable noise generation
        dac_disable(DAC1, DAC_CHANNEL1);
        optionFlags &= ~(1 << 1);
        millisec_timer_set_rel(&dac_reset_timer, MILLISEC_TIMER_NOW,
                               DAC_RESET_TIME_MS);
      }
      if (millisec_timer_is_expired(&dac_reset_timer, MILLISEC_TIMER_NOW)) {
        dac_enable(DAC1, DAC_CHANNEL1);
      }

      if (stream_state.on && !(optionFlags & 1) &&
          millisec_timer_is_expired_repeat(&adc_timer, MILLISEC_TIMER_NOW)) {
        adc_start_conversion_regular(ADC);
      }

      if (stream_state.on && circular_buffer_get_size_uint8(asc_out_buf) == 0) {
        if (optionFlags & 1) { // counter stream mode
          char counter_buffer[8];
          convert_uint32_to_hex(counter, counter_buffer, true);
          ascii_serial_com_device_put_s_message_in_output_buffer(
              &ascd, '0', '0', counter_buffer, 8);
          counter++;
        } else { // ADC stream mode
          if (adc_eos(ADC)) {
            if (adc_get_overrun_flag(ADC)) {
              adc_n_overruns++;
              adc_clear_overrun_flag(ADC);
            }
            static uint16_t adc_val;
            static char adc_val_buffer[4];
            adc_val = adc_read_regular(ADC);
            ADC_ISR(ADC) |= (1 << ADC_ISR_EOSEQ) | (1 << ADC_ISR_EOC);
            convert_uint16_to_hex(adc_val, adc_val_buffer, true);
            ascii_serial_com_device_put_s_message_in_output_buffer(
                &ascd, '0', '0', adc_val_buffer + 1, 3);
          }
        }
      }

      // Write data from asc_out_buf to serial
      if (!circular_buffer_is_empty_uint8(asc_out_buf) &&
          (USART_ISR(USART2) & USART_ISR_TXE)) {
        tmp_byte = circular_buffer_pop_front_uint8(asc_out_buf);
        usart_send(USART2, tmp_byte);
      }
    }
    Catch(e) { nExceptions++; }
  }

  return 0;
}

#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic push
void usart2_isr(void) {
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
  static uint16_t isr_tmp_byte;
  if (((USART_CR1(USART2) & USART_CR1_RXNEIE) != 0) &&
      ((USART_ISR(USART2) & USART_ISR_RXNE) != 0)) {
    isr_tmp_byte = usart_recv(USART2) & 0xFF;
    circular_buffer_push_back_uint8(&extraInputBuffer, isr_tmp_byte);
  }
}

void handle_nf_messages(__attribute__((unused)) ascii_serial_com *asc,
                        __attribute__((unused)) char ascVersion,
                        __attribute__((unused)) char appVersion, char command,
                        __attribute__((unused)) char *data,
                        __attribute__((unused)) size_t dataLen,
                        void *state_vp) {
  on_off_stream_state *state = (on_off_stream_state *)state_vp;
  if (command == 'n') {
    state->on = 1;
    millisec_timer_set_rel(&adc_timer, MILLISEC_TIMER_NOW,
                           adc_sample_period_ms);
  } else if (command == 'f') {
    state->on = 0;
  }
}
