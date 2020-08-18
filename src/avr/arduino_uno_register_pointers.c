#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_pointers.h"
#include "avr/avr_uart.h"
#include "avr/io.h"

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

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

CEXCEPTION_T e;

uint16_t nExceptions;

int main(void) {

  DDRB |= 1 << 5;

  nExceptions = 0;

  ascii_serial_com_register_pointers_init(&reg_pointers_state, regPtrs, masks,
                                          nRegs);
  ascii_serial_com_device_init(
      &ascd, ascii_serial_com_register_pointers_handle_message, NULL, NULL,
      &reg_pointers_state, NULL, NULL);
  circular_buffer_uint8 *asc_in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  circular_buffer_uint8 *asc_out_buf =
      ascii_serial_com_device_get_output_buffer(&ascd);

  USART0_Init(MYUBRR, 0);

  while (true) {
    Try {
      if (USART0_can_read_Rx_data) {
        circular_buffer_push_back_uint8(asc_in_buf, UDR0);
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
