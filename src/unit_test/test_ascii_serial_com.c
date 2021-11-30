#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "externals/unity.h"
#include <inttypes.h>
#include <stdio.h>

CEXCEPTION_T e1;
CEXCEPTION_T e2;
size_t iString;

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_convert_uint8_to_hex(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_convert_uint16_to_hex(void) {
  Try {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_convert_uint32_to_hex(void) {
  Try {
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
    for (uint64_t i = 0; i < 0x1F000; i++) {
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
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_convert_hex_to_uint8(void) {
  Try {
    char instr[5] = "\0\0\0\0\0";
    for (size_t i = 0; i < 0x100; i++) {
      snprintf(instr, 4, "%02" PRIX8, (uint8_t)i);
      // printf("%s    %02" PRIX8 "    %02"PRIX8"\n",instr, (uint8_t) instr[1],
      // (uint8_t) instr[0]);
      TEST_ASSERT_EQUAL_UINT8(i, convert_hex_to_uint8(instr));
      snprintf(instr, 4, "%02" PRIx8, (uint8_t)i);
      // printf("%s    %02" PRIX8 "    %02"PRIX8"\n",instr, (uint8_t) instr[1],
      // (uint8_t) instr[0]);
      TEST_ASSERT_EQUAL_UINT8(i, convert_hex_to_uint8(instr));
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_convert_hex_to_uint16(void) {
  Try {
    char instr[6] = "\0\0\0\0\0";
    for (size_t i = 0; i < 0x10000; i++) {
      snprintf(instr, 6, "%04" PRIX16, (uint16_t)i);
      TEST_ASSERT_EQUAL_UINT16(i, convert_hex_to_uint16(instr));
      snprintf(instr, 6, "%04" PRIx16, (uint16_t)i);
      TEST_ASSERT_EQUAL_UINT16(i, convert_hex_to_uint16(instr));
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_convert_hex_to_uint32(void) {
  Try {
    char instr[10] = "\0\0\0\0\0\0\0\0\0\0";
    for (uint64_t i = 0; i < 0x100000000; i += 0x100000) {
      snprintf(instr, 10, "%08" PRIX32, (uint32_t)i);
      TEST_ASSERT_EQUAL_UINT32(i, convert_hex_to_uint32(instr));
      snprintf(instr, 10, "%08" PRIx32, (uint32_t)i);
      TEST_ASSERT_EQUAL_UINT32(i, convert_hex_to_uint32(instr));
    }
    for (uint64_t i = 0; i < 0x1F000; i++) {
      snprintf(instr, 10, "%08" PRIX32, (uint32_t)i);
      TEST_ASSERT_EQUAL_UINT32(i, convert_hex_to_uint32(instr));
      snprintf(instr, 10, "%08" PRIx32, (uint32_t)i);
      TEST_ASSERT_EQUAL_UINT32(i, convert_hex_to_uint32(instr));
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_compute_checksum(void) {
  ascii_serial_com asc;
  ascii_serial_com_init(&asc);

  char checksumOut[5];
  checksumOut[4] = '\0'; // for easy printing

  Try {

    const char *strings[][2] = {
        {">xxx.", "79BD"},
        {">000.", "0FEC"},
        {">FFF.", "FD98"},
        {">1234567890ABCDEF.", "9411"},
        {">FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF.", "39DE"},
        {">00000000000000000000000000000000000000000.", "1DC1"},
    };
    for (iString = 0; iString < 6; iString++) {
      circular_buffer_clear_uint8(&asc.in_buf);
      circular_buffer_clear_uint8(&asc.out_buf);
      circular_buffer_push_back_string_uint8(&asc.in_buf, strings[iString][0]);
      circular_buffer_push_back_string_uint8(&asc.out_buf, strings[iString][0]);
      ascii_serial_com_compute_checksum(&asc, checksumOut, true);

      // printf("Message: \"%s\" checksums: Expected: %s, computed:
      // %s\n",strings[iString][0],strings[iString][1],checksumOut);
      for (size_t iByte = 0; iByte < 4; iByte++) {
        TEST_ASSERT_EQUAL_UINT8(strings[iString][1][iByte], checksumOut[iByte]);
      }
      ascii_serial_com_compute_checksum(&asc, checksumOut, false);
      // printf("Message: \"%s\" checksums: Expected: %s, computed:
      // %s\n",strings[iString][0],strings[iString][1],checksumOut);
      for (size_t iByte = 0; iByte < 4; iByte++) {
        TEST_ASSERT_EQUAL_UINT8(strings[iString][1][iByte], checksumOut[iByte]);
      }
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }

  const char *stringsInvalid[] = {".", "...."};
  for (iString = 0; iString < 2; iString++) {
    circular_buffer_clear_uint8(&asc.in_buf);
    circular_buffer_clear_uint8(&asc.out_buf);
    circular_buffer_push_back_string_uint8(&asc.in_buf,
                                           stringsInvalid[iString]);
    circular_buffer_push_back_string_uint8(&asc.out_buf,
                                           stringsInvalid[iString]);
    Try { ascii_serial_com_compute_checksum(&asc, checksumOut, true); }
    Catch(e2) { TEST_ASSERT_EQUAL(ASC_ERROR_INVALID_FRAME, e2); }
    Try { ascii_serial_com_compute_checksum(&asc, checksumOut, false); }
    Catch(e2) { TEST_ASSERT_EQUAL(ASC_ERROR_INVALID_FRAME, e2); }
  }

  const char *stringsInvalidPeriod[] = {">.",
                                        ">xx.",
                                        ".>",
                                        ".xxxxxx>",
                                        ">",
                                        ">>>>",
                                        ">asdgkjhq23kjhgqwerkjhg1234g\n"};
  for (iString = 0; iString < 7; iString++) {
    circular_buffer_clear_uint8(&asc.in_buf);
    circular_buffer_clear_uint8(&asc.out_buf);
    circular_buffer_push_back_string_uint8(&asc.in_buf,
                                           stringsInvalidPeriod[iString]);
    circular_buffer_push_back_string_uint8(&asc.out_buf,
                                           stringsInvalidPeriod[iString]);
    Try { ascii_serial_com_compute_checksum(&asc, checksumOut, true); }
    Catch(e2) { TEST_ASSERT_EQUAL(ASC_ERROR_INVALID_FRAME_PERIOD, e2); }
    Try { ascii_serial_com_compute_checksum(&asc, checksumOut, false); }
    Catch(e2) { TEST_ASSERT_EQUAL(ASC_ERROR_INVALID_FRAME_PERIOD, e2); }
  }
}

void test_ascii_serial_com_put_message_in_output_buffer(void) {
  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);

    const char *message1 = ">00w.23A6\n";
    size_t messageLen = 10;
    ascii_serial_com_put_message_in_output_buffer(&asc, '0', '0', 'w', "", 0);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message1[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);

    const char *message2 = ">00wFFFF.9F3B\n";
    messageLen = 14;
    ascii_serial_com_put_message_in_output_buffer(&asc, '0', '0', 'w', "FFFF",
                                                  4);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message2[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);

    const char *message3 =
        ">345666666666666666666666666666666666666666666666666666666.C7FB\n";
    const char *databuf = message3 + 4;
    messageLen = 64;
    ascii_serial_com_put_message_in_output_buffer(&asc, '3', '4', '5', databuf,
                                                  MAXDATALEN);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message3[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_get_message_from_input_buffer(void) {
  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *in_buf = ascii_serial_com_get_input_buffer(&asc);

    char ascVersion = '\0';
    char appVersion = '\0';
    char command = '\0';
    char data[MAXDATALEN + 1];
    for (size_t i = 0; i < MAXDATALEN + 1; i++) {
      data[i] = '\0';
    }
    size_t dataLen = 0;

    const char *message = ">abc.C103\n";
    circular_buffer_push_back_string_uint8(in_buf, message);
    ascii_serial_com_get_message_from_input_buffer(
        &asc, &ascVersion, &appVersion, &command, data, &dataLen);
    // printf("Message: \"%s\" ascVersion: %c appVersion: %c command: %c data:
    // %s dataLen: %zu\n", message, ascVersion, appVersion, command, data,
    // dataLen);
    TEST_ASSERT_EQUAL_UINT8('a', ascVersion);
    TEST_ASSERT_EQUAL_UINT8('b', appVersion);
    TEST_ASSERT_EQUAL_UINT8('c', command);
    TEST_ASSERT_EQUAL(0, dataLen);

    const char *message2 =
        ">defxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.350F\n";
    circular_buffer_push_back_string_uint8(in_buf, message2);
    ascii_serial_com_get_message_from_input_buffer(
        &asc, &ascVersion, &appVersion, &command, data, &dataLen);
    // printf("Message: \"%s\" ascVersion: %c appVersion: %c command: %c data:
    // %s dataLen: %zu\n", message2, ascVersion, appVersion, command, data,
    // dataLen);
    TEST_ASSERT_EQUAL_UINT8('d', ascVersion);
    TEST_ASSERT_EQUAL_UINT8('e', appVersion);
    TEST_ASSERT_EQUAL_UINT8('f', command);
    TEST_ASSERT_EQUAL(54, dataLen);
    for (size_t i = 0; i < dataLen; i++) {
      TEST_ASSERT_EQUAL_UINT8('x', data[i]);
    }

    const char *message3 = ">AFw0123456789.A86F\n";
    circular_buffer_push_back_string_uint8(in_buf, message3);
    ascii_serial_com_get_message_from_input_buffer(
        &asc, &ascVersion, &appVersion, &command, data, &dataLen);
    // printf("Message: \"%s\" ascVersion: %c appVersion: %c command: %c data:
    // %s dataLen: %zu\n", message3, ascVersion, appVersion, command, data,
    // dataLen);
    TEST_ASSERT_EQUAL_UINT8('A', ascVersion);
    TEST_ASSERT_EQUAL_UINT8('F', appVersion);
    TEST_ASSERT_EQUAL_UINT8('w', command);
    TEST_ASSERT_EQUAL(10, dataLen);
    for (size_t i = 0; i < dataLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(i + 0x30, data[i]);
    }
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_put_s_message_in_output_buffer(void) {
  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);

    const char *message1 = ">00s00,.7216\n";
    size_t messageLen = 13;
    ascii_serial_com_put_s_message_in_output_buffer(&asc, '0', '0', "", 0);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message1[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);

    const char *message2 = ">00s01,0000.91AE\n";
    size_t message2Len = 17;
    ascii_serial_com_put_s_message_in_output_buffer(&asc, '0', '0', "0000", 4);
    TEST_ASSERT_EQUAL_size_t(message2Len,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < message2Len; i++) {
      TEST_ASSERT_EQUAL_UINT8(message2[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);

    const char *message3 =
        ">00s02,000000000000000000000000000000000000000000000000000.7448\n";
    size_t message3Len = 64;
    ascii_serial_com_put_s_message_in_output_buffer(
        &asc, '0', '0', "000000000000000000000000000000000000000000000000000",
        51);
    TEST_ASSERT_EQUAL_size_t(message3Len,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < message3Len; i++) {
      TEST_ASSERT_EQUAL_UINT8(message3[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

void test_ascii_serial_com_put_error_in_output_buffer(void) {
  Try {
    ascii_serial_com asc;
    ascii_serial_com_init(&asc);
    circular_buffer_uint8 *out_buf = ascii_serial_com_get_output_buffer(&asc);

    const char *message1 = ">00eFF,w,.C117\n";
    size_t messageLen = 15;
    ascii_serial_com_put_error_in_output_buffer(&asc, '0', '0', 'w', "", 0,
                                                0xFF);
    // circular_buffer_print_uint8(out_buf,stderr,0);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message1[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);

    const char *message2 = ">1Fe05,e,0123.5368\n";
    messageLen = 19;
    ascii_serial_com_put_error_in_output_buffer(&asc, '1', 'F', 'e', "0123", 4,
                                                5);
    // circular_buffer_print_uint8(out_buf, stderr);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message2[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);

    const char *message3 = ">1FeFF,s,012345678.2FA0\n";
    messageLen = 24;
    ascii_serial_com_put_error_in_output_buffer(&asc, '1', 'F', 's',
                                                "0123456789ABCDEF", 16, 0xFF);
    // circular_buffer_print_uint8(out_buf, stderr);
    TEST_ASSERT_EQUAL_size_t(messageLen,
                             circular_buffer_get_size_uint8(out_buf));
    for (size_t i = 0; i < messageLen; i++) {
      TEST_ASSERT_EQUAL_UINT8(message3[i],
                              circular_buffer_get_element_uint8(out_buf, i));
    }
    circular_buffer_clear_uint8(out_buf);
  }
  Catch(e1) {
    printf("Uncaught exception: %u\n", e1);
    TEST_FAIL_MESSAGE("Uncaught exception!");
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_convert_uint8_to_hex);
  RUN_TEST(test_convert_uint16_to_hex);
  RUN_TEST(test_convert_uint32_to_hex);
  RUN_TEST(test_convert_hex_to_uint8);
  RUN_TEST(test_convert_hex_to_uint16);
  RUN_TEST(test_convert_hex_to_uint32);
  RUN_TEST(test_ascii_serial_com_compute_checksum);
  RUN_TEST(test_ascii_serial_com_put_message_in_output_buffer);
  RUN_TEST(test_ascii_serial_com_get_message_from_input_buffer);
  RUN_TEST(test_ascii_serial_com_put_s_message_in_output_buffer);
  RUN_TEST(test_ascii_serial_com_put_error_in_output_buffer);
  return UNITY_END();
}
