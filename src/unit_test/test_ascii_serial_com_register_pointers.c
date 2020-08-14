#include "asc_exception.h"
#include "ascii_serial_com_register_pointers.h"
#include "externals/unity.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

CEXCEPTION_T e1;
// CEXCEPTION_T e2;

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

#define nRegs 5
REGTYPE r1, r2, r3, r4, r5;
volatile REGTYPE *regPtrs[nRegs] = {&r1, &r2, &r3, &r4, &r5};
REGTYPE masks[nRegs] = {
    0, 0, 0, 0, 0,
};

void setUp(void) {
  // set stuff up here
  for (size_t i = 0; i < nRegs; i++) {
    *regPtrs[i] = 0;
    masks[i] = 0;
  }
}

void tearDown(void) {
  // clean stuff up here
}

void test_ascii_serial_com_register_pointers_write(void) {
  for (size_t iReg = 0; iReg < nRegs; iReg++) {
    masks[iReg] = 0xFF;
  }
  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);

    ascii_serial_com_register_pointers ascrb;
    ascii_serial_com_register_pointers_init(&ascrb, regPtrs, masks, nRegs);

    char ascVersion, appVersion, command, data[MAXDATALEN + 1];
    size_t dataLen;

    char validationStr[MAXMESSAGELEN + 1];
    for (size_t iReg = 0; iReg < nRegs; iReg++) {
      for (size_t iVal = 0; iVal < 0x100; iVal++) {
        // printf("iReg: %02zX iVal: %02zX\n",iReg,iVal);
        ascVersion = '0';
        appVersion = '0';
        command = 'w';
        sprintf(data, "%04zX," regFmtStr, iReg, iVal);
        dataLen = 5 + REGWIDTHBYTES * 2;
        sprintf(validationStr, ">%c%c%c%04zX.", ascVersion, appVersion, command,
                iReg);
        ascii_serial_com_register_pointers_handle_message(
            &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
        TEST_ASSERT_EQUAL_size_t(10 + 4,
                                 circular_buffer_get_size_uint8(out_buf));
        TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer, 5 + 4);
        circular_buffer_clear_uint8(out_buf);

        TEST_ASSERT_EQUAL_MESSAGE(iVal, *regPtrs[iReg],
                                  "Register didn't get set to correct value!");

        command = 'r';
        sprintf(data, "%04zX", iReg);
        dataLen = 4;
        sprintf(validationStr, ">%c%c%c%04zX," regFmtStr ".", ascVersion,
                appVersion, command, iReg, iVal);
        ascii_serial_com_register_pointers_handle_message(
            &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
        TEST_ASSERT_EQUAL_size_t(15 + REGWIDTHBYTES * 2,
                                 circular_buffer_get_size_uint8(out_buf));
        TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer,
                                     10 + REGWIDTHBYTES * 2);
        circular_buffer_clear_uint8(out_buf);
      }
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_register_pointers_write_masked(void) {
  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);

    ascii_serial_com_register_pointers ascrb;
    ascii_serial_com_register_pointers_init(&ascrb, regPtrs, masks, nRegs);

    char ascVersion, appVersion, command, data[MAXDATALEN + 1];
    size_t dataLen;

    char validationStr[MAXMESSAGELEN + 1];
    for (size_t iReg = 0; iReg < nRegs; iReg++) {
      masks[iReg] = 0x35;
    }
    for (size_t iReg = 0; iReg < nRegs; iReg++) {
      for (size_t iVal = 0; iVal < 0x100; iVal++) {
        // printf("iReg: %02zX iVal: %02zX\n",iReg,iVal);
        ascVersion = '0';
        appVersion = '0';
        command = 'w';
        sprintf(data, "%04zX," regFmtStr, iReg, iVal);
        dataLen = 5 + REGWIDTHBYTES * 2;
        sprintf(validationStr, ">%c%c%c%04zX.", ascVersion, appVersion, command,
                iReg);
        ascii_serial_com_register_pointers_handle_message(
            &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
        TEST_ASSERT_EQUAL_size_t(10 + 4,
                                 circular_buffer_get_size_uint8(out_buf));
        TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer, 5 + 4);
        circular_buffer_clear_uint8(out_buf);

        TEST_ASSERT_EQUAL_MESSAGE(iVal & 0x35, *regPtrs[iReg],
                                  "Register didn't get set to correct value!");

        command = 'r';
        sprintf(data, "%04zX", iReg);
        dataLen = 4;
        sprintf(validationStr, ">%c%c%c%04zX," regFmtStr ".", ascVersion,
                appVersion, command, iReg, iVal & 0x35);
        ascii_serial_com_register_pointers_handle_message(
            &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
        TEST_ASSERT_EQUAL_size_t(15 + REGWIDTHBYTES * 2,
                                 circular_buffer_get_size_uint8(out_buf));
        TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer,
                                     10 + REGWIDTHBYTES * 2);
        circular_buffer_clear_uint8(out_buf);
      }
    }
    for (size_t iReg = 0; iReg < nRegs; iReg++) {
      *regPtrs[iReg] = 0xFF;
    }
    for (size_t iReg = 0; iReg < nRegs; iReg++) {
      for (size_t iVal = 0; iVal < 0x100; iVal++) {
        // printf("iReg: %02zX iVal: %02zX\n",iReg,iVal);
        ascVersion = '0';
        appVersion = '0';
        command = 'w';
        sprintf(data, "%04zX," regFmtStr, iReg, iVal);
        dataLen = 5 + REGWIDTHBYTES * 2;
        sprintf(validationStr, ">%c%c%c%04zX.", ascVersion, appVersion, command,
                iReg);
        ascii_serial_com_register_pointers_handle_message(
            &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
        TEST_ASSERT_EQUAL_size_t(10 + 4,
                                 circular_buffer_get_size_uint8(out_buf));
        TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer, 5 + 4);
        circular_buffer_clear_uint8(out_buf);

        TEST_ASSERT_EQUAL_MESSAGE((iVal & 0x35) | (0xFF & ~0x35),
                                  *regPtrs[iReg],
                                  "Register didn't get set to correct value!");

        command = 'r';
        sprintf(data, "%04zX", iReg);
        dataLen = 4;
        sprintf(validationStr, ">%c%c%c%04zX," regFmtStr ".", ascVersion,
                appVersion, command, iReg, (iVal & 0x35) | (0xFF & ~0x35));
        ascii_serial_com_register_pointers_handle_message(
            &asc, ascVersion, appVersion, command, data, dataLen, &ascrb);
        TEST_ASSERT_EQUAL_size_t(15 + REGWIDTHBYTES * 2,
                                 circular_buffer_get_size_uint8(out_buf));
        TEST_ASSERT_EQUAL_CHAR_ARRAY(validationStr, out_buf->buffer,
                                     10 + REGWIDTHBYTES * 2);
        circular_buffer_clear_uint8(out_buf);
      }
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ascii_serial_com_register_pointers_write);
  RUN_TEST(test_ascii_serial_com_register_pointers_write_masked);
  return UNITY_END();
}
