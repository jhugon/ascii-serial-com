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

void test_millisec_timer_do_wrap_once(void) {
  millisec_timer timer;

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 1, MAXVAL);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      1, timer.set_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0, timer.expire_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 2));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, 2, MAXVAL);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      2, timer.set_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      1, timer.expire_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 2));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 3));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, MAXVAL - 1, 2);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      MAXVAL - 1, timer.set_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0, timer.expire_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  millisec_timer_set_rel(&timer, MAXVAL - 1, 2);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));
  millisec_timer_set_rel(&timer, MAXVAL - 1, 2);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL - 2));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 2));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, MAXVAL, 1);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      MAXVAL, timer.set_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      0, timer.expire_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  millisec_timer_set_rel(&timer, MAXVAL, 1);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));
  millisec_timer_set_rel(&timer, MAXVAL, 1);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL - 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 1));

  ////////////////////////////////////////////////////

  millisec_timer_set_rel(&timer, MAXVAL, 2);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      MAXVAL, timer.set_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      1, timer.expire_time,
      "Checking the test is what it should be, not an error with millisec "
      "timer but with the test");
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 0));
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 1));
  millisec_timer_set_rel(&timer, MAXVAL, 2);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, 2));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, 2));
  millisec_timer_set_rel(&timer, MAXVAL, 2);
  TEST_ASSERT_TRUE(millisec_timer_is_expired(&timer, MAXVAL - 1));
  TEST_ASSERT_FALSE(millisec_timer_is_expired(&timer, MAXVAL - 1));
}

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
