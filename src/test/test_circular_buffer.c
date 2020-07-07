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

void test_circular_buffer_push_pop_back_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  circular_buffer_push_back_uint8(&cb, 8);

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 1);

  TEST_ASSERT_EQUAL_UINT8(circular_buffer_pop_back_uint8(&cb), 8);

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 0);

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 5);

  for (uint8_t i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL_UINT8(circular_buffer_pop_back_uint8(&cb), 4 - i);
    TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 4 - i);
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 0);

  // printf("Should be empty:\n");
  // circular_buffer_print_uint8(&cb);

  for (uint8_t i = 0; i < 9; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 9);

  // printf("Should have 0 through 8:\n");
  // circular_buffer_print_uint8(&cb);

  circular_buffer_push_back_uint8(&cb, 9);

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 10);

  // printf("Should have 0 through 9:\n");
  // circular_buffer_print_uint8(&cb);

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(circular_buffer_pop_back_uint8(&cb), 9 - i);
    TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 9 - i);
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 0);
}

void test_circular_buffer_push_overfull_pop_back_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  for (uint8_t i = 0; i < 33; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 10);

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(circular_buffer_pop_back_uint8(&cb), 32 - i);
    TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 9 - i);
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  for (uint8_t i = 0; i < 29; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 10);

  for (uint8_t i = 0; i < 43; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 10);

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(circular_buffer_pop_back_uint8(&cb), 42 - i);
    TEST_ASSERT_EQUAL(circular_buffer_get_size_uint8(&cb), 9 - i);
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_circular_buffer_init_uint8);
  RUN_TEST(test_circular_buffer_push_pop_back_uint8);
  RUN_TEST(test_circular_buffer_push_overfull_pop_back_uint8);
  return UNITY_END();
}
