#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "avr/avr_uart.h"
#include "circular_buffer.h"

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

char dataBuffer[MAXDATALEN];
ascii_serial_com asc;

CEXCEPTION_T e;
bool rawLoopback;
char ascVersion, appVersion, command;
size_t dataLen;

uint16_t nExceptions;

int main(void) {

  nExceptions = 0;

  ascii_serial_com_init(&asc);
  circular_buffer_uint8 *asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

  USART0_Init(MYUBRR);

  while (true) {
    Try {

      if (USART0_can_read_Rx_data) {
        circular_buffer_push_back_uint8(asc_in_buf, UDR0);
      }

      if (!circular_buffer_is_empty_uint8(asc_in_buf)) {
        ascii_serial_com_get_message_from_input_buffer(
            &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
        if (command != '\0') {
          ascii_serial_com_put_message_in_output_buffer(
              &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
        }
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
