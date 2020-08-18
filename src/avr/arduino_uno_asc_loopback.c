#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "avr/avr_timers.h"
#include "avr/avr_uart.h"
#include "circular_buffer.h"

#include <avr/interrupt.h>
#include <stdio.h>
#include <util/atomic.h>

#define F_CPU 16000000L
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

// period = 1024*256*n_overflows / F_CPU
#define n_overflows F_CPU / 1024 / 256 / 2

#define NCHARINLINE 16

char dataBuffer[MAXDATALEN];
ascii_serial_com asc;

#define extraInputBuffer_size 64
uint8_t extraInputBuffer_raw[extraInputBuffer_size];
circular_buffer_uint8 extraInputBuffer;

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
  printf("\n  ");
  for (size_t i = 0; i < cb->capacity; i++) {
    char el = cb->buffer[i];
    if (el == '\n') {
      printf("\\n");
    } else {
      printf("%c", el);
    }
  }
  printf("\n  ");
  if (cb->iStart == cb->iStop) {
    for (size_t i = 0; i < cb->capacity; i++) {
      if (i == cb->iStart) {
        printf("X");
      } else {
        printf(" ");
      }
    }
  } else {
    for (size_t i = 0; i < cb->capacity; i++) {
      if (i == cb->iStart) {
        printf(">");
      } else if (i == cb->iStop) {
        printf("<");
      } else {
        printf(" ");
      }
    }
  }
  printf("\n");
}

static void print_memory(const char *title, void *address, size_t nBytes);
static void print_memory(const char *title, void *address, size_t nBytes) {
  const size_t nLines = nBytes / NCHARINLINE;
  printf("%s\n", title);
  for (size_t iLine = 0; iLine < nLines; iLine++) {
    printf("%p  ", address + iLine * NCHARINLINE);
    size_t nCharThisLine = NCHARINLINE;
    if (iLine == nLines - 1)
      nCharThisLine = nBytes % NCHARINLINE;
    for (size_t iChar = 0; iChar < nCharThisLine; iChar++) {
      printf("%02hhX", *((uint8_t *)(address + iLine * NCHARINLINE + iChar)));
    }
    printf("\n");
  }
}

int main(void) {

  nExceptions = 0;
  stdout = &mystdout;

  ascii_serial_com_init(&asc);
  // ascii_serial_com_set_ignore_CRC_mismatch(&asc);
  circular_buffer_uint8 *asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

  circular_buffer_init_uint8(&extraInputBuffer, extraInputBuffer_size,
                             extraInputBuffer_raw);

  USART0_Init(MYUBRR, 1);

  TIMER0_Init(5, 0, 1);
  DDRB |= _BV(5);

  sei();

  printf("####\n");
  printf("asc loc:                    %p\n", &asc);
  printf("asc_in_buf loc:             %p\n", asc_in_buf);
  printf("asc_out_buf loc:            %p\n", asc_out_buf);
  printf("asc->raw_buffer loc:        %p\n", &asc.raw_buffer);
  printf("asc->ignoreCRCMismatch loc: %p\n", &asc.ignoreCRCMismatch);
  printf("asc_in_buf->buffer loc:     %p\n", asc_in_buf->buffer);
  printf("asc_out_buf->buffer loc:    %p\n", asc_out_buf->buffer);
  printf("####\n");
  print_memory("ASC:", &asc, 256);
  printf("####\n");
  print_memory("asc.raw_buffer", asc.raw_buffer, 128);
  printf("####\n");
  print_memory("asc_in_buf.buffer", asc_in_buf->buffer, 64);
  printf("####\n");
  print_memory("asc_out_buf.buffer", asc_out_buf->buffer, 64);
  printf("####\n");

  while (true) {
    Try {

      // if (USART0_can_read_Rx_data) {
      //  uint8_t byte = UDR0;
      //  if (byte == 0x3F) { // '?'
      //    print_buffer("in", asc_in_buf);
      //    print_buffer("out", asc_out_buf);
      //  } else {
      //    circular_buffer_push_back_uint8(asc_in_buf, byte);
      //  }
      //}

      if (!circular_buffer_is_empty_uint8(&extraInputBuffer)) {
        uint8_t byte;
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
          byte = circular_buffer_pop_front_uint8(&extraInputBuffer);
        }
        if (byte == 0x3F) { // '?'
          print_buffer("in", asc_in_buf);
          print_buffer("out", asc_out_buf);
        } else {
          circular_buffer_push_back_uint8(asc_in_buf, byte);
        }
      }

      if (!circular_buffer_is_empty_uint8(asc_in_buf)) {
        ascii_serial_com_get_message_from_input_buffer(
            &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
        if (command != '\0') {
          printf("Got a message!:\n");
          print_buffer("in", asc_in_buf);
          ascii_serial_com_put_message_in_output_buffer(
              &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
          print_buffer("out", asc_out_buf);
        }
      }

      if (!circular_buffer_is_empty_uint8(asc_out_buf) &&
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

ISR(TIMER0_OVF_vect) {
  static uint16_t timer_overflow_count;
  timer_overflow_count += 1;
  if (timer_overflow_count == n_overflows) {
    PORTB ^= _BV(5); // flip bit
    timer_overflow_count = 0;
  }
}
