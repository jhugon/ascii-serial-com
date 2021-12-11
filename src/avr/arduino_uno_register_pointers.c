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

ascii_serial_com_device ascd;
ascii_serial_com_register_pointers reg_pointers_state;
ascii_serial_com_device_config ascd_config = {
    .func_rw = ascii_serial_com_register_pointers_handle_message,
    .state_rw = &reg_pointers_state};
circular_buffer_uint8 *asc_in_buf;
circular_buffer_uint8 *asc_out_buf;

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;

uint16_t nExceptions;

int main(void) {

  DDRB |= 1 << 5;

  nExceptions = 0;

  Try {
    ascii_serial_com_register_pointers_init(&reg_pointers_state, regPtrs, masks,
                                            nRegs);
    ascii_serial_com_device_init(&ascd, &ascd_config);
    asc_in_buf = ascii_serial_com_device_get_input_buffer(&ascd);
    asc_out_buf = ascii_serial_com_device_get_output_buffer(&ascd);

    circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                               extraInputBuffer_raw);
  }
  Catch(e) { return e; }

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
