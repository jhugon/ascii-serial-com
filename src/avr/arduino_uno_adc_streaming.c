#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "avr/avr_uart.h"
#include "circular_buffer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#define F_CPU 16000000L
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

uint16_t timer0B_counter;
uint16_t timer0B_counter_compare = 25;

// Lower byte of the 16 bit variables is in lower register number
#define nRegs 5
volatile REGTYPE *regPtrs[nRegs] = {
    &PORTB,
    &((uint8_t *)(&timer0B_counter))[0],
    &((uint8_t *)(&timer0B_counter))[1],
    &((uint8_t *)(&timer0B_counter_compare))[0],
    &((uint8_t *)(&timer0B_counter_compare))[1],
};

REGTYPE masks[nRegs] = {
    1 << 5, 0, 0, 0xFF, 0xFF,
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
char counter_buffer[2];

int main(void) {

  DDRB |= 1 << 5;

  // Use Tim0 interrupts to time ADC conversions
  // Tim0 in normal mode, which is default
  // Output compare is the interrupt
  // Bottom 3 bits of TCCR0B control clock prescaling
  //    0b101 = 0x5 is the lowest speed, clk/1024
  //    Setting this to != 0 is what enables the timer
  // The timer count can be access/modified at TCNT0
  // OCR0B sets the value for output compare unit B
  // TIMSK0: timer interrupt enable mask, bit OCIE0B enables output compare unit
  // Don't have to clear the interrupt flag manually, it's done automatically
  // It's the same with Tim1, it's just 16 bit
  TCNT0 = 0;
  OCR0B = F_CPU / 1024 / 100; // should be 100 times per second
  TIMSK0 |= 1 << OCIE0B; // output comapre interrupt enable for timer 0 unit B
  TCCR0B |= 0x5;         // enable timer with clk/1024

  nExceptions = 0;
  stream_state.on = 0;
  counter = 0;

  ascii_serial_com_register_pointers_init(&reg_pointers_state, regPtrs, masks,
                                          nRegs);
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

      if (stream_state.on && circular_buffer_get_size_uint8(asc_out_buf) == 0 &&
          timer0B_counter > timer0B_counter_compare) {
        convert_uint8_to_hex(counter, counter_buffer, true);
        ascii_serial_com_device_put_s_message_in_output_buffer(
            &ascd, '0', '0', counter_buffer, 2);
        counter++;
        ATOMIC_BLOCK(ATOMIC_FORCEON) { timer0B_counter = 0; }
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

ISR(TIMER0_COMPB_vect) { timer0B_counter++; }

void handle_nf_messages(__attribute__((unused)) ascii_serial_com *asc,
                        __attribute__((unused)) char ascVersion,
                        __attribute__((unused)) char appVersion, char command,
                        __attribute__((unused)) char *data,
                        __attribute__((unused)) size_t dataLen,
                        void *state_vp) {
  on_off_stream_state *state = (on_off_stream_state *)state_vp;
  if (command == 'n') {
    state->on = 1;
    TIMSK0 |= 1 << OCIE0B; // output comapre interrupt enable for timer 0 unit B
  } else if (command == 'f') {
    state->on = 0;
    TIMSK0 &= ~(1 << OCIE0B); // disable interrupt
  }
}
