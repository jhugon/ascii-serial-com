#include "ascii_serial_com_device.h"
#include "externals/unity.h"
#include <inttypes.h>
#include <stdio.h>

char ascVersion_f, appVersion_f, command_f;
char data_f[MAXDATALEN];
size_t dataLen_f;
void *state_f;

void testfunc(ascii_serial_com *asc, char ascVersion, char appVersion,
              char command, const char *data, size_t dataLen, void *state);

void testfunc(ascii_serial_com *asc, char ascVersion, char appVersion,
              char command, const char *data, size_t dataLen, void *state) {
  fprintf(stderr,
          "testfunc: asc: %p ascVersion %c appVersion %c command %c dataLen "
          "%zu state: %p\n  data: ",
          asc, ascVersion, appVersion, command, dataLen, state);
  for (size_t i = 0; i < dataLen; i++) {
    fprintf(stderr, "%c", data[i]);
    data_f[i] = data[i];
  }
  fprintf(stderr, "\n");
  fflush(stderr);
  ascVersion_f = ascVersion;
  appVersion_f = appVersion;
  command_f = command;
  dataLen_f = dataLen;
  state_f = state;
}

int state_rw;
int state_s;
int state_other;

void setUp(void) {
  // set stuff up here
  state_rw = 1;
  state_s = 2;
  state_other = 3;
}

void tearDown(void) {
  // clean stuff up here
}

void test_ascii_serial_com_device_receive_good(void) {
  ascii_serial_com_device ascd;
  ascii_serial_com_device_init(&ascd, testfunc, testfunc, testfunc, &state_rw,
                               &state_s, &state_other);
  circular_buffer_uint8 *in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  // circular_buffer_uint8* out_buf =
  // ascii_serial_com_device_get_output_buffer(&ascd);

  circular_buffer_push_back_string_uint8(in_buf, ">00w.23A6\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('0', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('0', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('w', command_f);
  TEST_ASSERT_EQUAL_size_t(0, dataLen_f);
  TEST_ASSERT_EQUAL(&state_rw, state_f);
  TEST_ASSERT_EQUAL_INT(state_rw, *((int *)state_f));

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">12r.4EBA\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('1', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('2', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('r', command_f);
  TEST_ASSERT_EQUAL_size_t(0, dataLen_f);
  TEST_ASSERT_EQUAL(&state_rw, state_f);
  TEST_ASSERT_EQUAL_INT(state_rw, *((int *)state_f));

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">00wFFFF.9F3B\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('0', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('0', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('w', command_f);
  TEST_ASSERT_EQUAL_size_t(4, dataLen_f);
  TEST_ASSERT_EQUAL_MEMORY("FFFF", data_f, 4);
  TEST_ASSERT_EQUAL(&state_rw, state_f);
  TEST_ASSERT_EQUAL_INT(state_rw, *((int *)state_f));

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">FFs111 222 333 444.B049\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('F', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('F', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('s', command_f);
  TEST_ASSERT_EQUAL_size_t(15, dataLen_f);
  TEST_ASSERT_EQUAL_MEMORY("111 222 333 444", data_f, dataLen_f);
  TEST_ASSERT_EQUAL(&state_s, state_f);
  TEST_ASSERT_EQUAL_INT(state_s, *((int *)state_f));

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(
      in_buf,
      ">345666666666666666666666666666666666666666666666666666666.C7FB\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('3', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('4', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('5', command_f);
  TEST_ASSERT_EQUAL_size_t(54, dataLen_f);
  TEST_ASSERT_EQUAL_MEMORY(
      "666666666666666666666666666666666666666666666666666666", data_f,
      dataLen_f);
  TEST_ASSERT_EQUAL(&state_other, state_f);
  TEST_ASSERT_EQUAL_INT(state_other, *((int *)state_f));
}

void test_ascii_serial_com_device_receive_null_state(void) {
  ascii_serial_com_device ascd;
  ascii_serial_com_device_init(&ascd, testfunc, testfunc, testfunc, NULL, NULL,
                               NULL);
  circular_buffer_uint8 *in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  // circular_buffer_uint8* out_buf =
  // ascii_serial_com_device_get_output_buffer(&ascd);

  circular_buffer_push_back_string_uint8(in_buf, ">00w.23A6\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('0', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('0', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('w', command_f);
  TEST_ASSERT_EQUAL_size_t(0, dataLen_f);
  TEST_ASSERT_EQUAL(NULL, state_f);

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">12r.4EBA\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('1', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('2', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('r', command_f);
  TEST_ASSERT_EQUAL_size_t(0, dataLen_f);
  TEST_ASSERT_EQUAL(NULL, state_f);

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">FFs111 222 333 444.B049\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('F', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('F', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('s', command_f);
  TEST_ASSERT_EQUAL_size_t(15, dataLen_f);
  TEST_ASSERT_EQUAL_MEMORY("111 222 333 444", data_f, dataLen_f);
  TEST_ASSERT_EQUAL(NULL, state_f);

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(
      in_buf,
      ">345666666666666666666666666666666666666666666666666666666.C7FB\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('3', ascVersion_f);
  TEST_ASSERT_EQUAL_CHAR('4', appVersion_f);
  TEST_ASSERT_EQUAL_CHAR('5', command_f);
  TEST_ASSERT_EQUAL_size_t(54, dataLen_f);
  TEST_ASSERT_EQUAL_MEMORY(
      "666666666666666666666666666666666666666666666666666666", data_f,
      dataLen_f);
  TEST_ASSERT_EQUAL(NULL, state_f);
}

void test_ascii_serial_com_device_receive_bad(void) {
  ascii_serial_com_device ascd;
  ascii_serial_com_device_init(&ascd, testfunc, testfunc, testfunc, &state_rw,
                               &state_s, &state_other);
  circular_buffer_uint8 *in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  // circular_buffer_uint8* out_buf =
  // ascii_serial_com_device_get_output_buffer(&ascd);

  command_f = '\n';
  circular_buffer_push_back_string_uint8(in_buf, "00w.23A6\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('\n', command_f);

  command_f = '\n';
  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">00wFFFF\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('\n', command_f);

  command_f = '\n';
  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('\n', command_f);

  command_f = '\n';
  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(
      in_buf,
      ">345666666666666666666666666666666666666666166666666666666.C7FB\n");
  ascii_serial_com_device_receive(&ascd);
  TEST_ASSERT_EQUAL_CHAR('\n', command_f);
}

void test_ascii_serial_com_device_receive_null_func(void) {
  ascii_serial_com_device ascd;
  ascii_serial_com_device_init(&ascd, NULL, NULL, NULL, NULL, NULL, NULL);
  circular_buffer_uint8 *in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  // circular_buffer_uint8* out_buf =
  // ascii_serial_com_device_get_output_buffer(&ascd);

  circular_buffer_push_back_string_uint8(in_buf, ">00w.23A6\n");
  ascii_serial_com_device_receive(&ascd);

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">12r.4EBA\n");
  ascii_serial_com_device_receive(&ascd);

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(in_buf, ">FFs111 222 333 444.B049\n");
  ascii_serial_com_device_receive(&ascd);

  circular_buffer_clear_uint8(in_buf);
  circular_buffer_push_back_string_uint8(
      in_buf,
      ">345666666666666666666666666666666666666666666666666666666.C7FB\n");
  ascii_serial_com_device_receive(&ascd);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ascii_serial_com_device_receive_good);
  RUN_TEST(test_ascii_serial_com_device_receive_null_state);
  RUN_TEST(test_ascii_serial_com_device_receive_bad);
  RUN_TEST(test_ascii_serial_com_device_receive_null_func);
  return UNITY_END();
}
