#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "avr/avr_uart.h"
#include "circular_buffer.h"

#include <stdio.h>

#define FOSC 16000000L
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

char dataBuffer[MAXDATALEN];
ascii_serial_com asc;

CEXCEPTION_T e;
char ascVersion, appVersion, command;
size_t dataLen;

uint16_t nExceptions;

static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
static int uart_putchar(char c, FILE *stream) {
  USART0_Tx(c);
  return 0;
}
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

static void print_buffer(const char *title, const circular_buffer_uint8 *cb);
static void print_buffer(const char *title, const circular_buffer_uint8 *cb) {
  printf("%s\n", title);
  const unsigned sz = circular_buffer_get_size_uint8(cb);
  printf("  size: %u\n  ", sz);
  for (size_t i = 0; i < sz; i++) {
    char el = circular_buffer_get_element_uint8(cb, i);
    if (el == '\n') {
      printf("\\n");
    } else {
      printf("%c", el);
    }
  }
  printf("\n");
}

#define NLINES 16
#define NCHARINLINE 16

int main(void) {

  nExceptions = 0;
  stdout = &mystdout;

  ascii_serial_com_init(&asc);
  ascii_serial_com_set_ignore_CRC_mismatch(&asc);
  circular_buffer_uint8 *asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

  USART0_Init(MYUBRR);

  printf("####\n");
  printf("asc loc:                    %p\n", &asc);
  printf("asc_in_buf loc:             %p\n", asc_in_buf);
  printf("asc_out_buf loc:            %p\n", asc_out_buf);
  printf("asc->raw_buffer loc:        %p\n", &asc.raw_buffer);
  printf("asc->ignoreCRCMismatch loc: %p\n", &asc.ignoreCRCMismatch);
  printf("asc_in_buf->buffer loc:     %p\n", asc_in_buf->buffer);
  printf("asc_out_buf->buffer loc:    %p\n", asc_out_buf->buffer);
  printf("####\n");
  for (size_t iLine = 0; iLine < NLINES; iLine++) {
    printf("%p  ", &asc + iLine * NCHARINLINE);
    for (size_t iChar = 0; iChar < NCHARINLINE; iChar++) {
      printf("%02hhX", *((uint8_t *)(&asc + iLine * NCHARINLINE + iChar)));
    }
    printf("\n");
  }
  printf("####\n");
  printf("asc.raw_buffer\n");
  for (size_t iLine = 0; iLine < 8; iLine++) {
    printf("%p  ", ((uint8_t *)asc.raw_buffer) + iLine * NCHARINLINE);
    for (size_t iChar = 0; iChar < NCHARINLINE; iChar++) {
      printf("%02hhX",
             *(((uint8_t *)asc.raw_buffer) + iLine * NCHARINLINE + iChar));
    }
    printf("\n");
  }
  printf("####\n");
  printf("asc_in_buf.buffer\n");
  for (size_t iLine = 0; iLine < 4; iLine++) {
    printf("%p  ", asc_in_buf->buffer + iLine * NCHARINLINE);
    for (size_t iChar = 0; iChar < NCHARINLINE; iChar++) {
      printf("%02hhX",
             *((uint8_t *)(asc_in_buf->buffer + iLine * NCHARINLINE + iChar)));
    }
    printf("\n");
  }
  printf("####\n");
  printf("asc_out_buf.buffer\n");
  for (size_t iLine = 0; iLine < 4; iLine++) {
    printf("%p  ", asc_out_buf->buffer + iLine * NCHARINLINE);
    for (size_t iChar = 0; iChar < NCHARINLINE; iChar++) {
      printf("%02hhX",
             *((uint8_t *)(asc_out_buf->buffer + iLine * NCHARINLINE + iChar)));
    }
    printf("\n");
  }
  printf("####\n");

  while (true) {
    Try {

      if (USART0_can_read_Rx_data) {
        circular_buffer_push_back_uint8(asc_in_buf, UDR0);
      }

      if (!circular_buffer_is_empty_uint8(asc_in_buf)) {
        ascii_serial_com_get_message_from_input_buffer(
            &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
        if (command != '\0') {
          print_buffer("in", asc_in_buf);
          ascii_serial_com_put_message_in_output_buffer(
              &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
          print_buffer("out", asc_out_buf);
        }
      }

      if (!circular_buffer_is_empty_uint8(asc_out_buf) > 0 &&
          USART0_can_write_Tx_data) {
        UDR0 = circular_buffer_pop_front_uint8(asc_out_buf);
      }
    }
    Catch(e) { nExceptions++; }
  }

  return 0;
}
