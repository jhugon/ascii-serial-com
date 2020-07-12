#include "ascii_serial_com.h"
#include "externals/unity.h"
#include <stdio.h>

static uint8_t fRead_mock_buf[100];
static size_t fRead_mock_iStart = 0;
static size_t fRead_mock_size = 0;
size_t fRead_mock(char *buf, size_t size);

#define fWrite_CAPACITY_MAX 100
static size_t fWrite_CAPACITY = fWrite_CAPACITY_MAX;
static uint8_t fWrite_mock_buf[fWrite_CAPACITY_MAX];
static size_t fWrite_mock_iStart = 0;
static size_t fWrite_mock_size = 0;
size_t fWrite_mock(const char *buf, size_t size);

size_t fRead_mock(char *buf, size_t size) {
  size_t write_size = fRead_mock_size;
  if (size < write_size) {
    write_size = size;
  }
  for (size_t iElement = fRead_mock_iStart; iElement < write_size; iElement++) {
    buf[iElement] = fRead_mock_buf[iElement];
  }
  fRead_mock_iStart += write_size;
  fRead_mock_size -= write_size;
  return write_size;
}

size_t fWrite_mock(const char *buf, size_t size) {
  size_t write_size = size;
  if (size >= fWrite_CAPACITY - fWrite_mock_iStart) {
    write_size = fWrite_CAPACITY - fWrite_mock_iStart;
  }
  if (size >= fWrite_CAPACITY_MAX - fWrite_mock_iStart) {
    write_size = fWrite_CAPACITY_MAX - fWrite_mock_iStart;
  }
  for (size_t iElement = 0; iElement < write_size; iElement++) {
    fWrite_mock_buf[fWrite_mock_iStart + iElement] = buf[iElement];
  }
  fWrite_mock_iStart += write_size;
  fWrite_mock_size += write_size;
  return write_size;
}

void setUp(void) {
  // set stuff up here

  fRead_mock_iStart = 0;
  fRead_mock_size = 0;

  fWrite_CAPACITY = fWrite_CAPACITY_MAX;
  fWrite_mock_iStart = 0;
  fWrite_mock_size = 0;
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

void test_ascii_serial_com_compute_checksum(void) {
  ascii_serial_com asc;
  ascii_serial_com_init(&asc, fRead_mock, fWrite_mock);

  char checksumOut[5];
  checksumOut[4] = '\0'; // for easy printing
  bool checksumFound;

  const char *stringsBad[] = {
      ">.",       ">xx.", ".>",
      ".xxxxxx>", ">",    ".",
      ">>>>",     "....", ">asdgkjhq23kjhgqwerkjhg1234g\n"};
  for (size_t iString = 0; iString < 9; iString++) {
    circular_buffer_clear_uint8(&asc.in_buf);
    circular_buffer_clear_uint8(&asc.out_buf);
    circular_buffer_push_back_string_uint8(&asc.in_buf, stringsBad[iString]);
    circular_buffer_push_back_string_uint8(&asc.out_buf, stringsBad[iString]);
    checksumFound = ascii_serial_com_compute_checksum(&asc, checksumOut, true);
    TEST_ASSERT_FALSE(checksumFound);
    checksumFound = ascii_serial_com_compute_checksum(&asc, checksumOut, false);
    TEST_ASSERT_FALSE(checksumFound);
  }

  const char *strings[][2] = {
      {">xxx.", "79BD"},
      {">000.", "0FEC"},
      {">FFF.", "FD98"},
      {">1234567890ABCDEF.", "9411"},
      {">FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF.", "39DE"},
      {">00000000000000000000000000000000000000000.", "1DC1"},
  };
  for (size_t iString = 0; iString < 6; iString++) {
    circular_buffer_clear_uint8(&asc.in_buf);
    circular_buffer_clear_uint8(&asc.out_buf);
    circular_buffer_push_back_string_uint8(&asc.in_buf, strings[iString][0]);
    circular_buffer_push_back_string_uint8(&asc.out_buf, strings[iString][0]);
    checksumFound = ascii_serial_com_compute_checksum(&asc, checksumOut, true);
    TEST_ASSERT_TRUE(checksumFound);

    // printf("Message: \"%s\" checksums: Expected: %s, computed:
    // %s\n",strings[iString][0],strings[iString][1],checksumOut);
    for (size_t iByte = 0; iByte < 4; iByte++) {
      TEST_ASSERT_EQUAL_UINT8(strings[iString][1][iByte], checksumOut[iByte]);
    }
    checksumFound = ascii_serial_com_compute_checksum(&asc, checksumOut, false);
    TEST_ASSERT_TRUE(checksumFound);
    // printf("Message: \"%s\" checksums: Expected: %s, computed:
    // %s\n",strings[iString][0],strings[iString][1],checksumOut);
    for (size_t iByte = 0; iByte < 4; iByte++) {
      TEST_ASSERT_EQUAL_UINT8(strings[iString][1][iByte], checksumOut[iByte]);
    }
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_convert_uint8_to_hex);
  RUN_TEST(test_convert_uint16_to_hex);
  RUN_TEST(test_convert_uint32_to_hex);
  RUN_TEST(test_ascii_serial_com_compute_checksum);
  return UNITY_END();
}
