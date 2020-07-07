#include "circular_buffer.h"
#include "externals/unity.h"
#include <stdio.h>

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_circular_buffer_init_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 0);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_circular_buffer_init_uint8);
  return UNITY_END();
}
