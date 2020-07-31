#include "ascii_serial_com_register_block.h"
#include "externals/unity.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#if REGWIDTHBYTES == 1
#define zerohex "00"
#define onehex "01"
#define maxhex "FF"
#define regFmtStr "%02zX"
#elif REGWIDTHBYTES == 2
#define zerohex "0000"
#define onehex "0001"
#define maxhex "FFFF"
#define regFmtStr "%04zX"
#elif REGWIDTHBYTES == 4
#define zerohex "00000000"
#define onehex "00000001"
#define maxhex "FFFFFFFF"
#define regFmtStr "%08zX"
#endif

#define nRegs 256
REGTYPE regs[nRegs];

void setUp(void) {
  // set stuff up here
  for (size_t i = 0; i < nRegs; i++) {
    regs[i] = 0;
  }
}

void tearDown(void) {
  // clean stuff up here
}

void test_ascii_serial_com_register_block_write(void) {
  ascii_serial_com asc;
  ascii_serial_com_init(&asc);
  circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);

  ascii_serial_com_register_block ascrb;
  ascii_serial_com_register_block_init(&ascrb, regs, nRegs);

  char ascVersion, appVersion, command, data[MAXDATALEN + 1];
  size_t dataLen;

  char validationStr[MAXMESSAGELEN + 1];
  for (size_t iReg = 0; iReg < 0x100; iReg++) {
    for (size_t iVal = 0; iVal < 0x100; iVal += 0x5) {
      // printf("iReg: %02zX iVal: %02zX\n",iReg,iVal);
      ascVersion = '0';
      appVersion = '0';
      command = 'w';
      sprintf(data, "%04zX," regFmtStr, iReg, iVal);
      dataLen = 5 + REGWIDTHBYTES * 2;
      sprintf(validationStr, ">%c%c%c%04zX.", ascVersion, appVersion, command,
              iReg);
      ascii_serial_com_register_block_handle_message(
          &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
      TEST_ASSERT_EQUAL_size_t(10 + 4, circular_buffer_get_size_uint8(out_buf));
      TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer, 5 + 4);
      circular_buffer_clear_uint8(out_buf);

      TEST_ASSERT_EQUAL_MESSAGE(iVal, regs[iReg],
                                "Register didn't get set to correct value!");

      command = 'r';
      sprintf(data, "%04zX", iReg);
      dataLen = 4;
      sprintf(validationStr, ">%c%c%c%04zX," regFmtStr ".", ascVersion,
              appVersion, command, iReg, iVal);
      ascii_serial_com_register_block_handle_message(
          &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
      TEST_ASSERT_EQUAL_size_t(15 + REGWIDTHBYTES * 2,
                               circular_buffer_get_size_uint8(out_buf));
      TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer,
                                   10 + REGWIDTHBYTES * 2);
      circular_buffer_clear_uint8(out_buf);
    }
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ascii_serial_com_register_block_write);
  return UNITY_END();
}
