#include "externals/unity.h"
#include "millisec_timer.h"
#include <stdio.h>

#define MAXVAL 0xFFFFFFFF

void setUp(void) {}

void tearDown(void) {}

void test_millisec_timer_not_wrap_once(void) {
  millisec_timer timer;

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 0, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 0));

  millisec_timer_set_rel(&timer, 0, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));

  millisec_timer_set_rel(&timer, 0, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 0, 1);
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));

  millisec_timer_set_rel(&timer, 0, 1);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));

  millisec_timer_set_rel(&timer, 0, 1);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 2));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 2));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 0, 5000);
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 5));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 4999));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 5000));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 5000));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 5001));
  millisec_timer_set_rel(&timer, 0, 5000);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 5000));
  millisec_timer_set_rel(&timer, 0, 5000);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 5001));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 0, MAXVAL - 1);
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 2));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL - 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  millisec_timer_set_rel(&timer, 0, MAXVAL - 1);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 0, MAXVAL);
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 2));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 1));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 5000, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  millisec_timer_set_rel(&timer, 5000, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  millisec_timer_set_rel(&timer, 5000, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 5000));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 5000, MAXVAL - 5000);
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 5000));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 1));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  millisec_timer_set_rel(&timer, 5000, MAXVAL - 5000);
  TEST_ASSERT_TRUE(
      millisec_timer_is_expired(&timer, 0)); // is sortof wrap around
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));

  ////////////////////////////////////////////////////
  // all sortof wrap around

  millisec_timer_set_rel(&timer, MAXVAL, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  millisec_timer_set_rel(&timer, MAXVAL, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  millisec_timer_set_rel(&timer, MAXVAL, 0);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 5000));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
}

void test_millisec_timer_do_wrap_once(void) {}

void test_millisec_timer_not_wrap_repeat(void) {}

void test_millisec_timer_do_wrap_repeat(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_millisec_timer_not_wrap_once);
  RUN_TEST(test_millisec_timer_do_wrap_once);
  RUN_TEST(test_millisec_timer_not_wrap_repeat);
  RUN_TEST(test_millisec_timer_do_wrap_repeat);
  return UNITY_END();
}
