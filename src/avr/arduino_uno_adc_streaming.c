/** \file
 *
 * \brief Uses timer to set interval between ADC single conversions that are
 * then sent as stream messages to host.
 *
 * On receiving 'n' message, begins streaming ADC values to host (by default the
 * ADC is connected to ground) Stops when 'f' message is received.
 *
 * ADC channel selection and time between ADC conversions are configurable with
 * registers.
 *
 * You can also turn on and off the user LED with register 0, bit 5
 *
 * Register map is documented at \ref register_map
 *
 */

#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "avr/avr_uart.h"
#include "circular_buffer.h"
#include "millisec_timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#define F_CPU 16000000L
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

bool have_started_ADC_conversion = false;
#define have_finished_ADC_conversion (!(ADCSRA & (1 << ADSC)))

millisec_timer adc_timer;
uint32_t adc_sample_period_ms = 1000;

#define nRegs 10

/** \brief Register Map
 *
 * Lower byte of the 16 bit variables is in lower register number
 *
 * |Register Number | Description | r/w | Default |
 * | -------------- |------------ | --- | ------- |
 * | 0 | PORTB, bit 5 is LED | r, bit 5 is w | 0 |
 * | 1 | ADMUX | r, lower 4 bits w | 0x4F (ground) |
 * | 2 | adc_sample_period_ms lowest 8 bits | r/w | 0xe8 (1000)|
 * | 3 | adc_sample_period_ms bits 8-15 | r/w | 0x03 |
 * | 4 | adc_sample_period_ms bits 16-23 | r/w | 0 |
 * | 5 | adc_sample_period_ms bits 24-21 | r/w | 0 |
 * | 6 | MILLISEC_TIMER_NOW lowest 8 bits | r | counting |
 * | 7 | MILLISEC_TIMER_NOW bits 8-15 | r | counting |
 * | 8 | MILLISEC_TIMER_NOW bits 16-23 | r | counting |
 * | 9 | MILLISEC_TIMER_NOW bits 24-21 | r | counting |
 *
 * ADMUX: the upper half of the register is the read only ADC reference
 * selection. The bottom 4 bits are r/w and are the channel selection.
 * values of 0-7 select the ADC channel to read from and 0xF is GND. **Don't
 * write any values besides 0-7 and 0xF, as it could cause issues.**
 *
 * adc_sample_period_ms: the time between ADC conversion
 * in milliseconds. default: 1000 ms = 1 s
 *
 * @see register_write_masks
 *
 */
volatile REGTYPE *register_map[nRegs] = {
    &PORTB,
    &ADMUX,
    &((uint8_t *)(&adc_sample_period_ms))[0],
    &((uint8_t *)(&adc_sample_period_ms))[1],
    &((uint8_t *)(&adc_sample_period_ms))[2],
    &((uint8_t *)(&adc_sample_period_ms))[3],
    &((uint8_t *)(&MILLISEC_TIMER_NOW))[0],
    &((uint8_t *)(&MILLISEC_TIMER_NOW))[1],
    &((uint8_t *)(&MILLISEC_TIMER_NOW))[2],
    &((uint8_t *)(&MILLISEC_TIMER_NOW))[3],
};

/** \brief Write masks for \ref register_map
 *
 * These define whether the given register in register_map is writable or not
 *
 */
REGTYPE register_write_masks[nRegs] = {
    1 << 5, // PORTB
    0x0F,   // ADMUX
    0xFF,   // adc_sample_period_ms
    0xFF,   // adc_sample_period_ms
    0xFF,   // adc_sample_period_ms
    0xFF,   // adc_sample_period_ms
    0,      // MILLISEC_TIMER_NOW
    0,      // MILLISEC_TIMER_NOW
    0,      // MILLISEC_TIMER_NOW
    0,      // MILLISEC_TIMER_NOW
};

typedef struct stream_state_struct {
  uint8_t on;
} on_off_stream_state;
void handle_nf_messages(ascii_serial_com *asc, char ascVersion, char appVersion,
                        char command, char *data, size_t dataLen,
                        void *state_vp);

on_off_stream_state stream_state;

ascii_serial_com_device ascd;
ascii_serial_com_register_pointers reg_pointers_state;
ascii_serial_com_device_config ascd_config = {
    .func_rw = ascii_serial_com_register_pointers_handle_message,
    .state_rw = &reg_pointers_state,
    .func_nf = handle_nf_messages,
    .state_nf = &stream_state};

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;

uint16_t nExceptions;
uint8_t counter;

int main(void) {

  DDRB |= 1 << 5;

  millisec_timer_avr_timer0_setup_16MHz();

  // ADC
  // ADMUX top 2 bits select reference. Default (0b00xxxxxx) is AREF--I think
  // that means whatever is applied to the AREF pin
  //    0b01xxxxxx selects AVCC (which is then connected to AREF so be careful
  //    with what's hooked there) 0b11xxxxxx selects internal 1.1V reference
  //    (which is then connected to AREF so be careful with what's hooked there)
  // ADMUX bottom 4 bits select channels 0 through 8 as just ints
  // ADMUX bottom 4 bits should be 0xF for ground and 0xE for bandgap (don't
  // use) ADCSRA Status and control reg A bits:
  //    ADEN: enable
  //    ADSC: start conversion; in single shot mode stays 1 until conversion
  //    complete ADIE: interrupt enable for conversion complete: ADC_vect Bottom
  //    2 bits are the ADC prescaler, they should divide the main clock to be
  //    between 50kHz and 200kHz
  //            for us at 16MHz, that means we need the /128 which is 0b111 = 7
  // ADC data for 16 bit reads is at "ADC"
  // DIDR0 diables digial inputs for ADC0-5 if they are being used
  ADMUX = 0x40;        // select AVCC as reference
  ADMUX |= 0xF;        // select ADC input
  ADCSRA = 7;          // set ADC clock prescaler
  ADCSRA |= 1 << ADEN; // enable ADC

  nExceptions = 0;
  stream_state.on = 0;
  counter = 0;

  ascii_serial_com_register_pointers_init(&reg_pointers_state, register_map,
                                          register_write_masks, nRegs);
  ascii_serial_com_device_init(&ascd, &ascd_config);
  circular_buffer_uint8 *asc_in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  circular_buffer_uint8 *asc_out_buf =
      ascii_serial_com_device_get_output_buffer(&ascd);

  circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                             extraInputBuffer_raw);

  USART0_Init(MYUBRR, 1);

  sei();

  while (true) {
    Try {
      // if (USART0_can_read_Rx_data) {
      //  circular_buffer_push_back_uint8(asc_in_buf, UDR0);
      //}

      if (!circular_buffer_is_empty_uint8(&extraInputBuffer)) {
        uint8_t byte;
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
          byte = circular_buffer_pop_front_uint8(&extraInputBuffer);
        }
        circular_buffer_push_back_uint8(asc_in_buf, byte);
      }

      ascii_serial_com_device_receive(&ascd);

      if (stream_state.on && !have_started_ADC_conversion &&
          millisec_timer_is_expired_repeat(&adc_timer, MILLISEC_TIMER_NOW)) {
        ADCSRA |= 1 << ADSC; // start ADC conversion
        have_started_ADC_conversion = true;
      }
      if (have_started_ADC_conversion && have_finished_ADC_conversion &&
          circular_buffer_get_size_uint8(asc_out_buf) == 0) {
        const uint16_t adc_val = ADC;
        char adc_val_buffer[4];
        convert_uint16_to_hex(adc_val, adc_val_buffer, true);
        ascii_serial_com_device_put_s_message_in_output_buffer(
            &ascd, '0', '0', adc_val_buffer + 1, 3);
        have_started_ADC_conversion = false;
      }

      if (circular_buffer_get_size_uint8(asc_out_buf) > 0 &&
          USART0_can_write_Tx_data) {
        UDR0 = circular_buffer_pop_front_uint8(asc_out_buf);
      }
    }
    Catch(e) { nExceptions++; }
  }

  return 0;
}

ISR(USART_RX_vect) {
  char c = UDR0;
  circular_buffer_push_back_uint8(&extraInputBuffer, c);
}

MILLISEC_TIMER_AVR_TIMER0_ISR;

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
