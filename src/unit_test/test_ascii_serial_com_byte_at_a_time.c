#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "externals/unity.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t input_str_size;
size_t output_str_size;
size_t i_input_str;
size_t i_output_str;
char input_str[MAXMESSAGELEN + 1];
char output_str[MAXMESSAGELEN + 1];

void send_out(uint8_t byte);
uint8_t receive_in(void);
bool anything_to_receive(void);
void null_inout_strs(void);

CEXCEPTION_T e1;
const char *thisMessage;

#define nMessages 12
const char *messages[nMessages] = {
    ">xxx.79BD\n",
    ">000.0FEC\n",
    ">FFF.FD98\n",
    ">1234567890ABCDEF.9411\n",
    ">FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF.39DE\n",
    ">00000000000000000000000000000000000000000.1DC1\n",
    ">345666666666666666666666666666666666666666666666666666666.C7FB\n",
    ">00wFFFF.9F3B\n",
    ">00w.23A6\n",
    ">abc.C103\n",
    ">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n",
    ">AFw0123456789.A86F\n"};

#define nBadMessages 6
const char *badMessages[nBadMessages] = {
    ">",           "\n", ">\n", ">abc\n", ">abcC103\n",
    ">abc.C104\n", // bad checksum
};

void send_out(uint8_t byte) { output_str[i_output_str++] = (char)byte; }
uint8_t receive_in(void) {
  if (anything_to_receive()) {
    return (uint8_t)input_str[i_input_str++];
  } else
    return 0xFF;
}

bool anything_to_receive(void) { return i_input_str < input_str_size; }

void setup_inout_strs(const char *instr) {
  i_input_str = 0;
  i_output_str = 0;
  output_str_size = 0;
  input_str_size = 0;
  for (size_t i = 0; i < MAXMESSAGELEN + 1; i++) {
    input_str[i] = '\0';
    output_str[i] = '\0';
  }
  strncpy(input_str, instr, MAXMESSAGELEN + 1);
  for (size_t i = 0; i < MAXMESSAGELEN + 1; i++) {
    if (input_str[i] == '\0')
      break;
    input_str_size++;
  }
}

void setUp(void) {
  // set stuff up here
  setup_inout_strs("");
}

void tearDown(void) {
  // clean stuff up here
}

void test_ascii_serial_com_loopback(void) {

  char ascVersion, appVersion, command, dataBuffer[MAXDATALEN];
  size_t dataLen;

  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *in_buf = ascii_serial_com_get_input_buffer(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);
    size_t iExtraIters = 0;

    for (size_t iMessage = 0; iMessage < nMessages; iMessage++) {
      setup_inout_strs(messages[iMessage]);
      while (anything_to_receive() ||
             !circular_buffer_is_empty_uint8(out_buf) || iExtraIters < 15) {
        if (anything_to_receive()) {
          circular_buffer_push_back_uint8(in_buf, receive_in());
        }
        if (!circular_buffer_is_empty_uint8(in_buf)) {
          ascii_serial_com_get_message_from_input_buffer(
              &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
          if (command != '\0') {
            ascii_serial_com_put_message_in_output_buffer(
                &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
          }
        }
        if (!circular_buffer_is_empty_uint8(out_buf)) {
          send_out(circular_buffer_pop_front_uint8(out_buf));
        }
        if (!(anything_to_receive() ||
              !circular_buffer_is_empty_uint8(out_buf))) {
          iExtraIters++;
        }
        // printf("######################################\n");
        // printf("iExtraIters: %zu\n",iExtraIters);
        // circular_buffer_print_uint8(in_buf,stdout);
        // circular_buffer_print_uint8(out_buf,stdout);
      }
      // printf("Expected: '%s'\n",messages[iMessage]);
      // printf("Observed: '%s'\n",output_str);
      TEST_ASSERT_EQUAL_STRING(messages[iMessage], output_str);
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_loopback_random_order(void) {

  char ascVersion, appVersion, command, dataBuffer[MAXDATALEN];
  size_t dataLen;

  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *in_buf = ascii_serial_com_get_input_buffer(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);
    size_t iExtraIters = 0;

    const size_t nTries = 200;
    for (size_t iTry = 0; iTry < nTries; iTry++) {
      const size_t iMessage = random() % nMessages;
      setup_inout_strs(messages[iMessage]);
      while (anything_to_receive() ||
             !circular_buffer_is_empty_uint8(out_buf) || iExtraIters < 15) {
        if (anything_to_receive()) {
          circular_buffer_push_back_uint8(in_buf, receive_in());
        }
        if (!circular_buffer_is_empty_uint8(in_buf)) {
          ascii_serial_com_get_message_from_input_buffer(
              &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
          if (command != '\0') {
            ascii_serial_com_put_message_in_output_buffer(
                &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
          }
        }
        if (!circular_buffer_is_empty_uint8(out_buf)) {
          send_out(circular_buffer_pop_front_uint8(out_buf));
        }
        if (!(anything_to_receive() ||
              !circular_buffer_is_empty_uint8(out_buf))) {
          iExtraIters++;
        }
        // printf("######################################\n");
        // printf("iExtraIters: %zu\n",iExtraIters);
        // circular_buffer_print_uint8(in_buf,stdout);
        // circular_buffer_print_uint8(out_buf,stdout);
      }
      // printf("Expected: '%s'\n",messages[iMessage]);
      // printf("Observed: '%s'\n",output_str);
      // circular_buffer_print_uint8(in_buf,stdout);
      // circular_buffer_print_uint8(out_buf,stdout);
      TEST_ASSERT_EQUAL_STRING(messages[iMessage], output_str);
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_loopback_random_order_bad_messages(void) {

  char ascVersion, appVersion, command, dataBuffer[MAXDATALEN];
  size_t dataLen;

  ascii_serial_com asc;
  ascii_serial_com_init(&asc);
  circular_buffer_uint8 *in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);
  size_t iExtraIters = 0;

  const size_t nTries = 200;
  for (size_t iTry = 0; iTry < nTries; iTry++) {
    bool isBadMessage = random() % 5000 < 500;
    if (iTry == nTries - 1)
      isBadMessage = false;
    if (isBadMessage) {
      const size_t iBadMessage = random() % nBadMessages;
      setup_inout_strs(badMessages[iBadMessage]);
      thisMessage = badMessages[iBadMessage];
    } else {
      const size_t iMessage = random() % nMessages;
      setup_inout_strs(messages[iMessage]);
      thisMessage = messages[iMessage];
    }
    while (anything_to_receive() || !circular_buffer_is_empty_uint8(out_buf) ||
           iExtraIters < 15) {
      Try {
        if (anything_to_receive()) {
          circular_buffer_push_back_uint8(in_buf, receive_in());
        }
        if (!circular_buffer_is_empty_uint8(in_buf)) {
          ascii_serial_com_get_message_from_input_buffer(
              &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
          if (command != '\0') {
            ascii_serial_com_put_message_in_output_buffer(
                &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
          }
        }
        if (!circular_buffer_is_empty_uint8(out_buf)) {
          send_out(circular_buffer_pop_front_uint8(out_buf));
        }
        if (!(anything_to_receive() ||
              !circular_buffer_is_empty_uint8(out_buf))) {
          iExtraIters++;
        }
        // printf("######################################\n");
        // printf("iExtraIters: %zu\n",iExtraIters);
        // circular_buffer_print_uint8(in_buf,stdout);
        // circular_buffer_print_uint8(out_buf,stdout);
      }
      Catch(e1) {
        if (e1 != 13) {
          printf("Uncaught exception: %u\n", e1);
          // TEST_FAIL_MESSAGE("Uncaught exception!");
        }
      }
    }
    if (isBadMessage) {
      TEST_ASSERT_EQUAL_STRING("", output_str);
    } else {
      // printf("Expected: '%s'\n",messages[iMessage]);
      // printf("Observed: '%s'\n",output_str);
      // circular_buffer_print_uint8(in_buf,stdout);
      // circular_buffer_print_uint8(out_buf,stdout);
      if (iTry == nTries - 1) {
        TEST_ASSERT_EQUAL_STRING(thisMessage, output_str);
      }
    }
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ascii_serial_com_loopback);
  RUN_TEST(test_ascii_serial_com_loopback_random_order);
  RUN_TEST(test_ascii_serial_com_loopback_random_order_bad_messages);
  return UNITY_END();
}
