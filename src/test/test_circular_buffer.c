#include "circular_buffer.h"
#include "externals/unity.h"
#include <stdio.h>

static uint8_t buf_mock[100];
static size_t buf_mock_iStart = 0;
static size_t buf_mock_size = 0;
size_t fRead_mock(uint8_t *buf, size_t size);

size_t fRead_mock(uint8_t *buf, size_t size) {
  size_t write_size = buf_mock_size;
  if (size < write_size) {
    write_size = size;
  }
  for (size_t iElement = buf_mock_iStart; iElement < write_size; iElement++) {
    buf[iElement] = buf_mock[iElement];
  }
  buf_mock_iStart += write_size;
  buf_mock_size -= write_size;
  return write_size;
}

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

void test_circular_buffer_get_first_block_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));
  const uint8_t *block = NULL;
  size_t blockSize = circular_buffer_get_first_block_uint8(&cb, &block);
  TEST_ASSERT_EQUAL(0, blockSize);

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }

  blockSize = circular_buffer_get_first_block_uint8(&cb, &block);
  TEST_ASSERT_EQUAL(5, blockSize);
  TEST_ASSERT_EQUAL_MESSAGE(cb.buffer, block,
                            "Pointers not equal cb buffer and block!");
  for (uint8_t i = 0; i < blockSize; i++) {
    TEST_ASSERT_EQUAL_UINT8(i, block[i]);
  }

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_pop_front_uint8(&cb);
  }
  TEST_ASSERT_EQUAL(5, cb.iStart);

  blockSize = circular_buffer_get_first_block_uint8(&cb, &block);
  TEST_ASSERT_EQUAL(0, blockSize);
  TEST_ASSERT_EQUAL_MESSAGE(cb.buffer + 5, block,
                            "Pointers not equal cb buffer and block!");

  for (uint8_t i = 0; i < 20; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  blockSize = circular_buffer_get_first_block_uint8(&cb, &block);
  TEST_ASSERT_EQUAL(5, blockSize);
  for (uint8_t i = 0; i < blockSize; i++) {
    TEST_ASSERT_EQUAL_UINT8(i + 10, block[i]);
  }
}

void test_circular_buffer_delete_first_block_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];

  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));
  size_t delSize = circular_buffer_delete_first_block_uint8(&cb);
  TEST_ASSERT_EQUAL(0, delSize);

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  delSize = circular_buffer_delete_first_block_uint8(&cb);
  TEST_ASSERT_EQUAL(5, delSize);

  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&cb, i);
    circular_buffer_pop_front_uint8(&cb);
  }
  for (uint8_t i = 0; i < 20; i++) {
    circular_buffer_push_back_uint8(&cb, i);
  }
  delSize = circular_buffer_delete_first_block_uint8(&cb);
  TEST_ASSERT_EQUAL(5, delSize);
  TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
  const uint8_t *block = NULL;
  size_t blockSize = circular_buffer_get_first_block_uint8(&cb, &block);
  TEST_ASSERT_EQUAL_MESSAGE(cb.buffer, block,
                            "Pointers not equal cb buffer and block!");
  for (uint8_t i = 0; i < blockSize; i++) {
    TEST_ASSERT_EQUAL_UINT8(i + 15, block[i]);
  }
  delSize = circular_buffer_delete_first_block_uint8(&cb);
  TEST_ASSERT_EQUAL(5, delSize);
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
}

void test_circular_buffer_push_back_block_uint8(void) {

  circular_buffer_uint8 cb;
  const size_t capacity = 10;
  uint8_t buf[capacity];
  circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

  buf_mock_size = 5;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }

  // circular_buffer_print_uint8(&cb);
  size_t nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(5, nPushed);
  TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

  buf_mock_size = 2;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }
  nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(2, nPushed);
  TEST_ASSERT_EQUAL(7, circular_buffer_get_size_uint8(&cb));

  buf_mock_size = 10;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }
  nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(3, nPushed);
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  // already full!
  buf_mock_size = 10;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }
  nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(0, nPushed);
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  // test just one empty at back
  circular_buffer_pop_back_uint8(&cb);

  buf_mock_size = 10;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }
  nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(1, nPushed);
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  // test just one empty at front
  circular_buffer_pop_front_uint8(&cb);

  buf_mock_size = 10;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }
  nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(1, nPushed);
  TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

  // Test start empty from middle
  for (uint8_t i = 0; i < 5; i++) {
    circular_buffer_pop_back_uint8(&cb);
    circular_buffer_pop_front_uint8(&cb);
  }
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

  buf_mock_size = 4;
  buf_mock_iStart = 0;
  for (uint8_t i = 0; i < buf_mock_size; i++) {
    buf_mock[i] = i;
  }
  nPushed = circular_buffer_push_back_block_uint8(&cb, fRead_mock);
  // circular_buffer_print_uint8(&cb);
  TEST_ASSERT_EQUAL(4, nPushed);
  TEST_ASSERT_EQUAL(4, circular_buffer_get_size_uint8(&cb));
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
  RUN_TEST(test_circular_buffer_get_first_block_uint8);
  RUN_TEST(test_circular_buffer_delete_first_block_uint8);
  RUN_TEST(test_circular_buffer_push_back_block_uint8);
  return UNITY_END();
}
