#include "compute_crc.h"

// Automatically generated CRC function
// polynomial: 0x13D65, bit reverse algorithm
// From crcmod predefined CRC-16-DNP
uint16_t computeCRC_16_DNP(uint8_t *data, size_t len, uint16_t crc) {
  static const uint16_t table[256] = {
      0x0000U, 0x365EU, 0x6CBCU, 0x5AE2U, 0xD978U, 0xEF26U, 0xB5C4U, 0x839AU,
      0xFF89U, 0xC9D7U, 0x9335U, 0xA56BU, 0x26F1U, 0x10AFU, 0x4A4DU, 0x7C13U,
      0xB26BU, 0x8435U, 0xDED7U, 0xE889U, 0x6B13U, 0x5D4DU, 0x07AFU, 0x31F1U,
      0x4DE2U, 0x7BBCU, 0x215EU, 0x1700U, 0x949AU, 0xA2C4U, 0xF826U, 0xCE78U,
      0x29AFU, 0x1FF1U, 0x4513U, 0x734DU, 0xF0D7U, 0xC689U, 0x9C6BU, 0xAA35U,
      0xD626U, 0xE078U, 0xBA9AU, 0x8CC4U, 0x0F5EU, 0x3900U, 0x63E2U, 0x55BCU,
      0x9BC4U, 0xAD9AU, 0xF778U, 0xC126U, 0x42BCU, 0x74E2U, 0x2E00U, 0x185EU,
      0x644DU, 0x5213U, 0x08F1U, 0x3EAFU, 0xBD35U, 0x8B6BU, 0xD189U, 0xE7D7U,
      0x535EU, 0x6500U, 0x3FE2U, 0x09BCU, 0x8A26U, 0xBC78U, 0xE69AU, 0xD0C4U,
      0xACD7U, 0x9A89U, 0xC06BU, 0xF635U, 0x75AFU, 0x43F1U, 0x1913U, 0x2F4DU,
      0xE135U, 0xD76BU, 0x8D89U, 0xBBD7U, 0x384DU, 0x0E13U, 0x54F1U, 0x62AFU,
      0x1EBCU, 0x28E2U, 0x7200U, 0x445EU, 0xC7C4U, 0xF19AU, 0xAB78U, 0x9D26U,
      0x7AF1U, 0x4CAFU, 0x164DU, 0x2013U, 0xA389U, 0x95D7U, 0xCF35U, 0xF96BU,
      0x8578U, 0xB326U, 0xE9C4U, 0xDF9AU, 0x5C00U, 0x6A5EU, 0x30BCU, 0x06E2U,
      0xC89AU, 0xFEC4U, 0xA426U, 0x9278U, 0x11E2U, 0x27BCU, 0x7D5EU, 0x4B00U,
      0x3713U, 0x014DU, 0x5BAFU, 0x6DF1U, 0xEE6BU, 0xD835U, 0x82D7U, 0xB489U,
      0xA6BCU, 0x90E2U, 0xCA00U, 0xFC5EU, 0x7FC4U, 0x499AU, 0x1378U, 0x2526U,
      0x5935U, 0x6F6BU, 0x3589U, 0x03D7U, 0x804DU, 0xB613U, 0xECF1U, 0xDAAFU,
      0x14D7U, 0x2289U, 0x786BU, 0x4E35U, 0xCDAFU, 0xFBF1U, 0xA113U, 0x974DU,
      0xEB5EU, 0xDD00U, 0x87E2U, 0xB1BCU, 0x3226U, 0x0478U, 0x5E9AU, 0x68C4U,
      0x8F13U, 0xB94DU, 0xE3AFU, 0xD5F1U, 0x566BU, 0x6035U, 0x3AD7U, 0x0C89U,
      0x709AU, 0x46C4U, 0x1C26U, 0x2A78U, 0xA9E2U, 0x9FBCU, 0xC55EU, 0xF300U,
      0x3D78U, 0x0B26U, 0x51C4U, 0x679AU, 0xE400U, 0xD25EU, 0x88BCU, 0xBEE2U,
      0xC2F1U, 0xF4AFU, 0xAE4DU, 0x9813U, 0x1B89U, 0x2DD7U, 0x7735U, 0x416BU,
      0xF5E2U, 0xC3BCU, 0x995EU, 0xAF00U, 0x2C9AU, 0x1AC4U, 0x4026U, 0x7678U,
      0x0A6BU, 0x3C35U, 0x66D7U, 0x5089U, 0xD313U, 0xE54DU, 0xBFAFU, 0x89F1U,
      0x4789U, 0x71D7U, 0x2B35U, 0x1D6BU, 0x9EF1U, 0xA8AFU, 0xF24DU, 0xC413U,
      0xB800U, 0x8E5EU, 0xD4BCU, 0xE2E2U, 0x6178U, 0x5726U, 0x0DC4U, 0x3B9AU,
      0xDC4DU, 0xEA13U, 0xB0F1U, 0x86AFU, 0x0535U, 0x336BU, 0x6989U, 0x5FD7U,
      0x23C4U, 0x159AU, 0x4F78U, 0x7926U, 0xFABCU, 0xCCE2U, 0x9600U, 0xA05EU,
      0x6E26U, 0x5878U, 0x029AU, 0x34C4U, 0xB75EU, 0x8100U, 0xDBE2U, 0xEDBCU,
      0x91AFU, 0xA7F1U, 0xFD13U, 0xCB4DU, 0x48D7U, 0x7E89U, 0x246BU, 0x1235U,
  };

  crc = crc ^ 0xFFFFU;
  while (len > 0) {
    crc = table[*data ^ (uint8_t)crc] ^ (crc >> 8);
    data++;
    len--;
  }
  crc = crc ^ 0xFFFFU;
  return crc;
}
