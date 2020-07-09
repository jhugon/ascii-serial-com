#include "ascii_serial_com.h"
#include "externals/unity.h"
#include <stdio.h>

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_convert_uint8_to_hex(void) {
  char stdiostr[4];
  char outstr[3] = "\0\0\0";
  for (size_t i = 0; i < 0x100; i++) {
    char j = i;
    convert_uint8_to_hex(j, outstr, true);
    snprintf(stdiostr, 4, "%02hhX", j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 2);

    convert_uint8_to_hex(j, outstr, false);
    snprintf(stdiostr, 4, "%02hhx", j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 2);
  }
}

void test_convert_uint16_to_hex(void) {
  char stdiostr[6];
  char outstr[5] = "\0\0\0\0\0";
  for (size_t i = 0; i < 0x10000; i++) {
    uint16_t j = i;
    convert_uint16_to_hex(j, outstr, true);
    snprintf(stdiostr, 6, "%04" PRIX16, j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 4);

    convert_uint16_to_hex(j, outstr, false);
    snprintf(stdiostr, 6, "%04" PRIx16, j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 4);
  }
}

void test_convert_uint32_to_hex(void) {
  char stdiostr[10];
  char outstr[9];
  for (size_t i = 0; i < 9; i++) {
    outstr[i] = '\0';
  }
  for (uint64_t i = 0; i < 0x100000000; i += 0x100000) {
    uint32_t j = i;
    convert_uint32_to_hex(j, outstr, true);
    snprintf(stdiostr, 10, "%08" PRIX32, j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 8);

    convert_uint32_to_hex(j, outstr, false);
    snprintf(stdiostr, 10, "%08" PRIx32, j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 8);
  }
  for (uint64_t i = 0; i < 0x10010; i++) {
    uint32_t j = i;
    convert_uint32_to_hex(j, outstr, true);
    snprintf(stdiostr, 10, "%08" PRIX32, j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 8);

    convert_uint32_to_hex(j, outstr, false);
    snprintf(stdiostr, 10, "%08" PRIx32, j);
    // printf("%zu: %s = %s\n",i,stdiostr,outstr);
    TEST_ASSERT_EQUAL_MEMORY(stdiostr, outstr, 8);
  }
}

void test_ascii_serial_com_init(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ascii_serial_com_init);
  RUN_TEST(test_convert_uint8_to_hex);
  RUN_TEST(test_convert_uint16_to_hex);
  RUN_TEST(test_convert_uint32_to_hex);
  return UNITY_END();
}
