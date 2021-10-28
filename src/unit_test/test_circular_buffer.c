#include "asc_exception.h"
#include "circular_buffer.h"
#include "externals/unity.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

CEXCEPTION_T e1;
// CEXCEPTION_T e2;

static uint8_t buf_mock[100];
static size_t buf_mock_size = 0;

void setUp(void) {
  // set stuff up here
  buf_mock_size = 0;
}

void tearDown(void) {
  // clean stuff up here
}

void test_circular_buffer_init_uint8(void) {
  Try {
    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];

    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

    TEST_ASSERT_TRUE(circular_buffer_is_empty_uint8(&cb));
    TEST_ASSERT_FALSE(circular_buffer_is_full_uint8(&cb));
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_print_uint8(void) {
  Try {
    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];

    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

    FILE *outfile = fopen("/dev/null", "a");
    if (!outfile) {
      perror("Couldn't open /dev/null");
      TEST_FAIL_MESSAGE("Couldn't open output file /dev/null");
    }

    circular_buffer_print_uint8(&cb, outfile, 2);
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_pop_back_uint8(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_overfull_pop_back_uint8(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_pop_front_uint8(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_overfull_pop_front_uint8(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_pop_front_back_uint8(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_get_element_uint8(void) {
  Try {
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

    // TEST_ASSERT_EQUAL_UINT8(0,
    // circular_buffer_get_element_uint8(&cb,7)); // to test
    // exception handling
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_remove_front_to_uint8(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_remove_back_to_uint8(void) {
  Try {

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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_find_first_uint8(void) {
  Try {

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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_find_last_uint8(void) {
  Try {

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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_count_uint8(void) {
  Try {

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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_get_first_block_uint8(void) {
  Try {

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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_delete_first_block_uint8(void) {
  Try {

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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_back_block_uint8(void) {
  Try {

    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];
    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

    buf_mock_size = 5;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }

    // circular_buffer_print_uint8(&cb);
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

    buf_mock_size = 2;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(7, circular_buffer_get_size_uint8(&cb));

    buf_mock_size = 10;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

    // already full!
    buf_mock_size = 10;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

    // test just one empty at back
    circular_buffer_pop_back_uint8(&cb);

    buf_mock_size = 10;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

    // test just one empty at front
    circular_buffer_pop_front_uint8(&cb);

    buf_mock_size = 10;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));

    // Test start empty from middle
    for (uint8_t i = 0; i < 5; i++) {
      circular_buffer_pop_back_uint8(&cb);
      circular_buffer_pop_front_uint8(&cb);
    }
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    buf_mock_size = 4;
    for (uint8_t i = 0; i < buf_mock_size; i++) {
      buf_mock[i] = i;
    }
    circular_buffer_push_back_block_uint8(&cb, buf_mock, buf_mock_size);
    // circular_buffer_print_uint8(&cb);
    TEST_ASSERT_EQUAL(4, circular_buffer_get_size_uint8(&cb));
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_pop_front_block_uint8(void) {
  Try {

    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];
    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

    uint8_t outputBuffer[100];

    for (uint8_t i = 0; i < 5; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    size_t nPopped =
        circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 100);
    // printf("outputBuffer: ");
    // for (uint8_t i = 0; i < 5; i++) {
    //  printf("%hhu",outputBuffer[i]);
    //}
    // printf("\n");
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      // printf("Byte number: %zu is
      // %hhu\n",i,outputBuffer[5]);
      TEST_ASSERT_EQUAL_UINT8(i, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 5; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 0);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));

    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 5);
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 0);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 5; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 0);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 2);
    TEST_ASSERT_EQUAL(2, nPopped);
    for (uint8_t i = 0; i < 2; i++) {
      TEST_ASSERT_EQUAL_UINT8(i, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(3, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 1);
    TEST_ASSERT_EQUAL(1, nPopped);
    TEST_ASSERT_EQUAL_UINT8(2, outputBuffer[0]);
    TEST_ASSERT_EQUAL(2, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 5);
    TEST_ASSERT_EQUAL(2, nPopped);
    for (uint8_t i = 0; i < 2; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 3, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 5);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    /////////////////////

    for (uint8_t i = 0; i < 10; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 0);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 100);
    TEST_ASSERT_EQUAL(10, nPopped);
    for (uint8_t i = 0; i < 10; i++) {
      TEST_ASSERT_EQUAL_UINT8(i, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 10; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 10);
    TEST_ASSERT_EQUAL(10, nPopped);
    for (uint8_t i = 0; i < 10; i++) {
      TEST_ASSERT_EQUAL_UINT8(i, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 10; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 9);
    TEST_ASSERT_EQUAL(9, nPopped);
    for (uint8_t i = 0; i < 9; i++) {
      TEST_ASSERT_EQUAL_UINT8(i, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(1, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 9);
    TEST_ASSERT_EQUAL(1, nPopped);
    TEST_ASSERT_EQUAL_UINT8(9, outputBuffer[0]);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 5);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    ////////////////////////////////////////////
    // test if buffer wraps around end of memory
    for (uint8_t i = 0; i < 15; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 0);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 100);
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 5, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 100);
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 10, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 15; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 10);
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 5, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 100);
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 10, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 15; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_TRUE(circular_buffer_is_full_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 9);
    TEST_ASSERT_EQUAL(5, nPopped);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 5, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 4);
    TEST_ASSERT_EQUAL(4, nPopped);
    for (uint8_t i = 0; i < 4; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 10, outputBuffer[i]);
    }
    TEST_ASSERT_EQUAL(1, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 9);
    TEST_ASSERT_EQUAL(1, nPopped);
    TEST_ASSERT_EQUAL_UINT8(14, outputBuffer[0]);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
    nPopped = circular_buffer_pop_front_block_uint8(&cb, outputBuffer, 5);
    TEST_ASSERT_EQUAL(0, nPopped);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_push_back_from_fd_uint8(void) {
  Try {

    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];
    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));

    const int infd = open("/dev/zero", O_RDONLY);
    if (infd == -1) {
      perror("Couldn't open /dev/zero");
      TEST_FAIL_MESSAGE("Couldn't open input "
                        "file /dev/zero");
    }

    size_t nRead = circular_buffer_push_back_from_fd_uint8(&cb, infd);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_EQUAL(10, nRead);
    for (uint8_t i = 0; i < 10; i++) {
      TEST_ASSERT_EQUAL_UINT8(0, circular_buffer_pop_front_uint8(&cb));
    }

    for (uint8_t i = 0; i < 5; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    nRead = circular_buffer_push_back_from_fd_uint8(&cb, infd);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_EQUAL(5, nRead);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i, circular_buffer_pop_front_uint8(&cb));
    }
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(0, circular_buffer_pop_front_uint8(&cb));
    }

    for (uint8_t i = 0; i < 15; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    nRead = circular_buffer_push_back_from_fd_uint8(&cb, infd);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    TEST_ASSERT_EQUAL(5, nRead);
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 10, circular_buffer_pop_front_uint8(&cb));
    }
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(0, circular_buffer_pop_front_uint8(&cb));
    }

    // setup so that iStart > iStop
    for (uint8_t i = 0; i < 12; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    for (uint8_t i = 0; i < 3; i++) {
      circular_buffer_pop_front_uint8(&cb);
    }
    TEST_ASSERT_EQUAL(7, circular_buffer_get_size_uint8(&cb));
    nRead = circular_buffer_push_back_from_fd_uint8(&cb, infd);
    TEST_ASSERT_EQUAL(3, nRead);
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    for (uint8_t i = 0; i < 7; i++) {
      TEST_ASSERT_EQUAL_UINT8(5 + i, circular_buffer_pop_front_uint8(&cb));
    }
    for (uint8_t i = 0; i < 3; i++) {
      TEST_ASSERT_EQUAL_UINT8(0, circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_pop_front_to_fd_uint8(void) {
  Try {

    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];
    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    const int outfd = open("/dev/null", O_WRONLY | O_APPEND);
    if (outfd == -1) {
      perror("Couldn't open /dev/null");
      TEST_FAIL_MESSAGE("Couldn't open output file "
                        "/dev/null");
    }

    size_t nWritten = circular_buffer_pop_front_to_fd_uint8(&cb, outfd);
    TEST_ASSERT_EQUAL(0, nWritten);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 1; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(1, circular_buffer_get_size_uint8(&cb));
    nWritten = circular_buffer_pop_front_to_fd_uint8(&cb, outfd);
    TEST_ASSERT_EQUAL(1, nWritten);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 10; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    nWritten = circular_buffer_pop_front_to_fd_uint8(&cb, outfd);
    TEST_ASSERT_EQUAL(10, nWritten);
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    for (uint8_t i = 0; i < 15; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    TEST_ASSERT_EQUAL(10, circular_buffer_get_size_uint8(&cb));
    nWritten = circular_buffer_pop_front_to_fd_uint8(&cb, outfd);
    TEST_ASSERT_EQUAL(5, nWritten);
    TEST_ASSERT_EQUAL(5, circular_buffer_get_size_uint8(&cb));
    for (uint8_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_UINT8(10 + i, circular_buffer_pop_front_uint8(&cb));
    }

    // setup so that iStart > iStop
    circular_buffer_clear_uint8(&cb);
    for (uint8_t i = 0; i < 12; i++) {
      circular_buffer_push_back_uint8(&cb, i);
    }
    for (uint8_t i = 0; i < 3; i++) {
      circular_buffer_pop_front_uint8(&cb);
    }
    TEST_ASSERT_EQUAL(7, circular_buffer_get_size_uint8(&cb));
    nWritten = circular_buffer_pop_front_to_fd_uint8(&cb, outfd);
    TEST_ASSERT_EQUAL(5, nWritten);
    TEST_ASSERT_EQUAL(2, circular_buffer_get_size_uint8(&cb));
    for (uint8_t i = 0; i < 2; i++) {
      TEST_ASSERT_EQUAL_UINT8(10 + i, circular_buffer_pop_front_uint8(&cb));
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_circular_buffer_remove_front_unfinished_frames_uint8(void) {
  Try {
    circular_buffer_uint8 cb;
    const size_t capacity = 10;
    uint8_t buf[capacity];
    circular_buffer_init_uint8(&cb, capacity, (uint8_t *)(&buf));
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    const char *testString = "\nxxxxa";
    size_t resultSize = 0;
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));

    testString = "\nxxxxa>abcdefg";
    resultSize = 8;
    const char *resultString = ">abcdefg";
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    testString = "\nxa>abc\n";
    resultSize = 5;
    resultString = ">abc\n";
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    testString = "ab\nxa>abc\n";
    resultSize = 5;
    resultString = ">abc\n";
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    testString = "abcdefg";
    resultSize = 0;
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    testString = ">a>b>c>d";
    resultSize = 2;
    resultString = ">d";
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    testString = ">a>b>c>d";
    resultSize = 2;
    resultString = ">d";
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));

    testString = "c>d>abc\n";
    resultSize = 5;
    resultString = ">abc\n";
    circular_buffer_push_back_string_uint8(&cb, testString);
    circular_buffer_remove_front_unfinished_frames_uint8(&cb, '>', '\n');
    circular_buffer_print_uint8(&cb, stderr, 2);
    TEST_ASSERT_EQUAL(resultSize, circular_buffer_get_size_uint8(&cb));
    for (size_t i = 0; i < resultSize; i++) {
      TEST_ASSERT_EQUAL_CHAR(resultString[i],
                             circular_buffer_pop_front_uint8(&cb));
    }
    TEST_ASSERT_EQUAL(0, circular_buffer_get_size_uint8(&cb));
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_circular_buffer_init_uint8);
  RUN_TEST(test_circular_buffer_print_uint8);
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
  RUN_TEST(test_circular_buffer_pop_front_block_uint8);
  RUN_TEST(test_circular_buffer_push_back_from_fd_uint8);
  RUN_TEST(test_circular_buffer_pop_front_to_fd_uint8);
  RUN_TEST(test_circular_buffer_remove_front_unfinished_frames_uint8);
  return UNITY_END();
}
