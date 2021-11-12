#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_block.h"
#include "avr/avr_uart.h"
#include "circular_buffer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#define F_CPU 16000000L
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

char dataBuffer[MAXDATALEN];
#define nRegs 10
REGTYPE regs[nRegs];

ascii_serial_com_device ascd;
ascii_serial_com_register_block reg_block;

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

CEXCEPTION_T e;

uint16_t nExceptions;

int main(void) {

  nExceptions = 0;

  ascii_serial_com_register_block_init(&reg_block, regs, nRegs);
  ascii_serial_com_device_init(&ascd,
                               ascii_serial_com_register_block_handle_message,
                               NULL, NULL, NULL, &reg_block, NULL, NULL, NULL);
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
