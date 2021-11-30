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

char dataBuffer[MAXDATALEN];
#define nRegs 3
volatile REGTYPE *regPtrs[nRegs] = {&PORTB, &PORTC, &PORTD};

REGTYPE masks[nRegs] = {
    1 << 5,
    0,
    0,
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

      if (stream_state.on && circular_buffer_get_size_uint8(asc_out_buf) == 0) {
        convert_uint8_to_hex(counter, counter_buffer, true);
        ascii_serial_com_device_put_s_message_in_output_buffer(
            &ascd, '0', '0', counter_buffer, 2);
        counter++;
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

void handle_nf_messages(__attribute__((unused)) ascii_serial_com *asc,
                        __attribute__((unused)) char ascVersion,
                        __attribute__((unused)) char appVersion, char command,
                        __attribute__((unused)) char *data,
                        __attribute__((unused)) size_t dataLen,
                        void *state_vp) {
  on_off_stream_state *state = (on_off_stream_state *)state_vp;
  if (command == 'n') {
    state->on = 1;
  } else if (command == 'f') {
    state->on = 0;
  }
}
