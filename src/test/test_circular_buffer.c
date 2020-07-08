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
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
}

void test_circular_buffer_push_pop_back_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  circular_buffer_push_back_uint8(&cb, 8);

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(1, circular_buffer_get_size_uint8(&cb));

  TEST_ASSERT_EQUAL_UINT8(8, circular_buffer_pop_back_uint8(&cb));

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL_UINT8(4 - i, circular_buffer_pop_back_uint8(&cb));
    TEST_ASSERT_EQUAL(4 - i, circular_buffer_get_size_uint8(&cb));
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

  // printf("Should be empty:\n");
  // circular_buffer_print_uint8(&cb);

  for (uint8_t i = 0; i < 9; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(9, circular_buffer_get_size_uint8(&cb));

  // printf("Should have 0 through 8:\n");
  // circular_buffer_print_uint8(&cb);

  circular_buffer_push_back_uint8(&cb, 9);

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  // printf("Should have 0 through 9:\n");
  // circular_buffer_print_uint8(&cb);

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(9 - i, circular_buffer_pop_back_uint8(&cb));
    TEST_ASSERT_EQUAL(9 - i, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
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
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(32 - i, circular_buffer_pop_back_uint8(&cb));
    TEST_ASSERT_EQUAL(9 - i, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  for (uint8_t i = 0; i < 29; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 43; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(42 - i, circular_buffer_pop_back_uint8(&cb));
    TEST_ASSERT_EQUAL(9 - i, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
}

void test_circular_buffer_push_pop_front_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  circular_buffer_push_front_uint8(&cb, 8);

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(1, circular_buffer_get_size_uint8(&cb));

  TEST_ASSERT_EQUAL_UINT8(8, circular_buffer_pop_front_uint8(&cb));

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL_UINT8(4 - i, circular_buffer_pop_front_uint8(&cb));
    TEST_ASSERT_EQUAL(4 - i, circular_buffer_get_size_uint8(&cb));
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

  // printf("Should be empty:\n");
  // circular_buffer_print_uint8(&cb);

  for (uint8_t i = 0; i < 9; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(9, circular_buffer_get_size_uint8(&cb));

  // printf("Should have 0 through 8:\n");
  // circular_buffer_print_uint8(&cb);

  circular_buffer_push_front_uint8(&cb, 9);

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  // printf("Should have 0 through 9:\n");
  // circular_buffer_print_uint8(&cb);

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(9 - i, circular_buffer_pop_front_uint8(&cb));
    TEST_ASSERT_EQUAL(9 - i, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
}

void test_circular_buffer_push_overfull_pop_front_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  for (uint8_t i = 0; i < 33; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(32 - i, circular_buffer_pop_front_uint8(&cb));
    TEST_ASSERT_EQUAL(9 - i, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  for (uint8_t i = 0; i < 29; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 43; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  TEST_ASSERT_FALSE(circular_buffer_is_empty_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL_UINT8(42 - i, circular_buffer_pop_front_uint8(&cb));
    TEST_ASSERT_EQUAL(9 - i, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
}

void test_circular_buffer_push_pop_front_back_uint8(void) {
  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  for (uint8_t i = 0; i < 2; i++) {
    TEST_ASSERT_EQUAL_UINT8(4 - i, circular_buffer_pop_front_uint8(&cb));
    TEST_ASSERT_EQUAL(4 - i, circular_buffer_get_size_uint8(&cb));
  }

  circular_buffer_push_back_uint8(&cb, 111);
  circular_buffer_push_back_uint8(&cb, 222);
  circular_buffer_push_back_uint8(&cb, 255);
  TEST_ASSERT_EQUAL(6, circular_buffer_get_size_uint8(&cb));

  TEST_ASSERT_EQUAL_UINT8(255, circular_buffer_pop_back_uint8(&cb));
  TEST_ASSERT_EQUAL_UINT8(222, circular_buffer_pop_back_uint8(&cb));
  TEST_ASSERT_EQUAL_UINT8(111, circular_buffer_pop_back_uint8(&cb));
  TEST_ASSERT_EQUAL_UINT8(0, circular_buffer_pop_back_uint8(&cb));
  TEST_ASSERT_EQUAL_UINT8(1, circular_buffer_pop_back_uint8(&cb));
  TEST_ASSERT_EQUAL(1, circular_buffer_get_size_uint8(&cb));
  TEST_ASSERT_EQUAL_UINT8(2, circular_buffer_pop_back_uint8(&cb));
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
}

void test_circular_buffer_get_element_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }

  for (uint8_t i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL_UINT8(4 - i, circular_buffer_get_element_uint8(&cb, i));
  }

  // TEST_ASSERT_EQUAL_UINT8(0, circular_buffer_get_element_uint8(&cb,7)); // to
  // test exception handling
}

void test_circular_buffer_remove_front_to_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  for (uint8_t i = 0; i < 8; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  circular_buffer_remove_front_to_uint8(&cb, 4, true);
  TEST_ASSERT_EQUAL(3, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 3; i++) {
    circular_buffer_push_back_uint8(&cb, 55);
  }

  circular_buffer_remove_front_to_uint8(&cb, 6, false);
  TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 6; i < 8; i++) {
    TEST_ASSERT_EQUAL_UINT8(i, circular_buffer_pop_front_uint8(&cb));
  }

  for (uint8_t i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL_UINT8(55, circular_buffer_pop_front_uint8(&cb));
  }
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  circular_buffer_remove_front_to_uint8(&cb, 6, true);
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  for (uint8_t i = 0; i < 20; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  circular_buffer_remove_front_to_uint8(&cb, 0, true);
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  circular_buffer_remove_front_to_uint8(&cb, 0, true);
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
}

void test_circular_buffer_remove_back_to_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  for (uint8_t i = 0; i < 8; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  circular_buffer_remove_back_to_uint8(&cb, 4, true);
  TEST_ASSERT_EQUAL(4, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 3; i++) {
    circular_buffer_push_back_uint8(&cb, 55);
  }

  circular_buffer_remove_back_to_uint8(&cb, 2, false);
  TEST_ASSERT_EQUAL(3, circular_buffer_get_size_uint8(&cb));

  for (uint8_t i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL_UINT8(i, circular_buffer_pop_front_uint8(&cb));
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
  circular_buffer_remove_back_to_uint8(&cb, 6, true);
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  for (uint8_t i = 0; i < 20; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  circular_buffer_remove_back_to_uint8(&cb, 0, true);
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));

  circular_buffer_remove_back_to_uint8(&cb, 0, true);
  TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
}

void test_circular_buffer_find_first_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  TEST_ASSERT_TRUE_MESSAGE(circular_buffer_find_first_uint8(&cb, 3) >=
                               circular_buffer_get_size_uint8(&cb),
                           "find_first should be >= size but isn't");

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_TRUE_MESSAGE(circular_buffer_find_first_uint8(&cb, 8) >=
                               circular_buffer_get_size_uint8(&cb),
                           "find_first should be >= size but isn't");
  TEST_ASSERT_EQUAL(3, circular_buffer_find_first_uint8(&cb, 3));

  for (uint8_t i = 0; i < 2; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(5, circular_buffer_find_first_uint8(&cb, 3));

  for (uint8_t i = 0; i < 50; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(5, circular_buffer_find_first_uint8(&cb, 45));

  for (uint8_t i = 0; i < 10; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(3, circular_buffer_find_first_uint8(&cb, 3));
  circular_buffer_push_back_uint8(&cb, 3);
  TEST_ASSERT_EQUAL(2, circular_buffer_find_first_uint8(&cb, 3));
  circular_buffer_push_front_uint8(&cb, 3);
  TEST_ASSERT_EQUAL(0, circular_buffer_find_first_uint8(&cb, 3));
}

void test_circular_buffer_find_last_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  TEST_ASSERT_TRUE_MESSAGE(circular_buffer_find_last_uint8(&cb, 3) >=
                               circular_buffer_get_size_uint8(&cb),
                           "find_last should be >= size but isn't");

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_TRUE_MESSAGE(circular_buffer_find_last_uint8(&cb, 8) >=
                               circular_buffer_get_size_uint8(&cb),
                           "find_last should be >= size but isn't");
  TEST_ASSERT_EQUAL(3, circular_buffer_find_last_uint8(&cb, 3));

  for (uint8_t i = 0; i < 2; i++) {
    circular_buffer_push_front_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(5, circular_buffer_find_last_uint8(&cb, 3));

  for (uint8_t i = 0; i < 50; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(5, circular_buffer_find_last_uint8(&cb, 45));

  for (uint8_t i = 0; i < 10; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(3, circular_buffer_find_last_uint8(&cb, 3));
  circular_buffer_push_front_uint8(&cb, 3);
  TEST_ASSERT_EQUAL(4, circular_buffer_find_last_uint8(&cb, 3));
  circular_buffer_push_back_uint8(&cb, 3);
  TEST_ASSERT_EQUAL(9, circular_buffer_find_last_uint8(&cb, 3));
}

void test_circular_buffer_count_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  TEST_ASSERT_EQUAL(0, circular_buffer_count_uint8(&cb, 0));
  TEST_ASSERT_EQUAL(0, circular_buffer_count_uint8(&cb, 5));

  for (uint8_t i = 0; i < 10; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  TEST_ASSERT_EQUAL(0, circular_buffer_count_uint8(&cb, 10));
  TEST_ASSERT_EQUAL(1, circular_buffer_count_uint8(&cb, 9));
  TEST_ASSERT_EQUAL(1, circular_buffer_count_uint8(&cb, 4));
  TEST_ASSERT_EQUAL(1, circular_buffer_count_uint8(&cb, 0));
  circular_buffer_push_back_uint8(&cb, 0);
  TEST_ASSERT_EQUAL(1, circular_buffer_count_uint8(&cb, 0));
  circular_buffer_push_back_uint8(&cb, 5);
  TEST_ASSERT_EQUAL(2, circular_buffer_count_uint8(&cb, 5));
  for (uint8_t i = 0; i < 50; i++) {
    circular_buffer_push_back_uint8(&cb, 255);
  }
  TEST_ASSERT_EQUAL(10, circular_buffer_count_uint8(&cb, 255));
  for (uint8_t i = 0; i < 10; i++) {
    circular_buffer_pop_back_uint8(&cb);
  }
  TEST_ASSERT_EQUAL(0, circular_buffer_count_uint8(&cb, 255));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_circular_buffer_init_uint8);
  RUN_TEST(test_circular_buffer_push_pop_back_uint8);
  RUN_TEST(test_circular_buffer_push_overfull_pop_back_uint8);
  RUN_TEST(test_circular_buffer_push_pop_front_uint8);
  RUN_TEST(test_circular_buffer_push_overfull_pop_front_uint8);
  RUN_TEST(test_circular_buffer_push_pop_front_back_uint8);
  RUN_TEST(test_circular_buffer_get_element_uint8);
  RUN_TEST(test_circular_buffer_remove_front_to_uint8);
  RUN_TEST(test_circular_buffer_remove_back_to_uint8);
  RUN_TEST(test_circular_buffer_find_first_uint8);
  RUN_TEST(test_circular_buffer_find_last_uint8);
  RUN_TEST(test_circular_buffer_count_uint8);
  return UNITY_END();
}
