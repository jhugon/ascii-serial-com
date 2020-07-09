#include "ascii_serial_com.h"
#include "assert.h"

void convert_uint8_to_hex(uint8_t num, char *outstr, bool caps) {
  for (uint8_t i = 0; i < 2; i++) {
    uint8_t nibble = (num >> (4 * i)) & 0xF;
    if (nibble < 10) {
      outstr[1 - i] = 0x30 + nibble;
    } else if (caps) {
      outstr[1 - i] = 0x41 + nibble - 10;
    } else {
      outstr[1 - i] = 0x61 + nibble - 10;
    }
  }
}

void convert_uint16_to_hex(uint16_t num, char *outstr, bool caps) {
  for (uint8_t i = 0; i < 2; i++) {
    convert_uint8_to_hex((num >> (8 * i)) & 0xFF, outstr + (1 - i) * 2, caps);
  }
}

void convert_uint32_to_hex(uint32_t num, char *outstr, bool caps) {
  for (uint8_t i = 0; i < 4; i++) {
    convert_uint8_to_hex((num >> (8 * i)) & 0xFF, outstr + (3 - i) * 2, caps);
  }
}

uint8_t convert_hex_to_uint8(char *instr) {
  uint8_t result = 0;
  for (uint8_t i = 0; i < 2; i++) {
    char thischar = instr[1 - i];
    if (thischar >= 0x30 && thischar <= 0x39) {
      result |= (thischar - 0x30) << (i * 4);
    } else if (thischar >= 0x41 && thischar <= 0x46) {
      result |= (thischar - 0x41 + 10) << (i * 4);
    } else if (thischar >= 0x61 && thischar <= 0x66) {
      result |= (thischar - 0x41 + 10) << (i * 4);
    } else {
      assert(false);
    }
  }
  return result;
}

uint16_t convert_hex_to_uint16(char *instr) {
  uint16_t result = 0;
  for (uint8_t i = 0; i < 2; i++) {
    result |= convert_hex_to_uint8(instr + (1 - i) * 2) << (8 * i);
  }
  return result;
}

uint32_t convert_hex_to_uint32(char *instr) {
  uint32_t result = 0;
  for (uint8_t i = 0; i < 4; i++) {
    result |= convert_hex_to_uint8(instr + (3 - i) * 2) << (8 * i);
  }
  return result;
}
