#include "ascii_serial_com.h"

#include <assert.h>

///////////////////////////////////////////////////////////////////////////

void ascii_serial_com_init(ascii_serial_com *asc, uint8_t registerBitWidth,
                           char appVersion, char asciiSerialComVersion,
                           size_t (*fRead)(char *, size_t),
                           size_t (*fWrite)(char *, size_t)) {

  asc->registerBitWidth = registerBitWidth;
  asc->registerByteWidth = registerBitWidth >> 3; // divide by 8
  if ((registerBitWidth & 0x7) > 0) {             // if mod 8 > 0
    asc->registerByteWidth++;
  }
  asc->appVersion = appVersion;
  asc->asciiSerialComVersion = asciiSerialComVersion;
  asc->fRead = fRead;
  asc->fWrite = fWrite;
  circular_buffer_init_uint8(&(asc->in_buf), MAXMESSAGELEN, asc->raw_buffer);
  circular_buffer_init_uint8(&(asc->out_buf), MAXMESSAGELEN,
                             asc->raw_buffer + MAXMESSAGELEN);
}

/////////////////////////////////////////////////////

void ascii_serial_com_pack_message_push_out(ascii_serial_com *asc, char command,
                                            char *data, size_t dataLen) {
  assert(dataLen < MAXMESSAGELEN);
  circular_buffer_push_back_uint8(&asc->out_buf, '>');
  circular_buffer_push_back_uint8(&asc->out_buf, asc->asciiSerialComVersion);
  circular_buffer_push_back_uint8(&asc->out_buf, asc->appVersion);
  circular_buffer_push_back_uint8(&asc->out_buf, command);
  for (size_t i = 0; i < dataLen; i++) {
    circular_buffer_push_back_uint8(&asc->out_buf, data[i]);
  }
  circular_buffer_push_back_uint8(&asc->out_buf, '.');
  char checksum[NCHARCHECKSUM];
  assert(ascii_serial_com_compute_checksum(asc, checksum, true));
  for (size_t i = 0; i < NCHARCHECKSUM; i++) {
    circular_buffer_push_back_uint8(&asc->out_buf, checksum[i]);
  }
  circular_buffer_push_back_uint8(&asc->out_buf, '\n');
}

void ascii_serial_com_pop_in_unpack(ascii_serial_com *asc, char *ascVersion,
                                    char *appVersion, char *command, char *data,
                                    size_t *dataLen) {

  char computeChecksum[NCHARCHECKSUM];
  circular_buffer_remove_front_to_uint8(&asc->in_buf, '>', false);
  if (!ascii_serial_com_compute_checksum(asc, computeChecksum, false)) {
    // invalid checksum, so probably couldn't find a valid message
    *command = '\0';
    *dataLen = 0;
    return;
  }
  *ascVersion = circular_buffer_pop_front_uint8(&asc->in_buf);
  *appVersion = circular_buffer_pop_front_uint8(&asc->in_buf);
  *command = circular_buffer_pop_front_uint8(&asc->in_buf);
  *dataLen = 0;
  for (size_t iData = 0; iData < MAXDATALEN; iData++) {
    char dataByte = circular_buffer_pop_front_uint8(&asc->in_buf);
    if (dataByte == '.') {
      break;
    }
    data[iData] = dataByte;
    *dataLen += 1;
  }
  if (*dataLen == MAXDATALEN &&
      circular_buffer_pop_front_uint8(&asc->in_buf) != '.') {
    // never found '.', so bad message
    *command = '\0';
    *dataLen = 0;
    return;
  }
  char receiveChecksum[NCHARCHECKSUM];
  for (size_t iChk = 0; iChk < NCHARCHECKSUM; iChk++) {
    receiveChecksum[iChk] = circular_buffer_pop_front_uint8(&asc->in_buf);
  }
  for (size_t iChk = 0; iChk < NCHARCHECKSUM; iChk++) {
    if (receiveChecksum[iChk] == computeChecksum[iChk]) {
      // checksum mismatch!
      *command = '\0';
      *dataLen = 0;
      return;
    }
  }
  return; // success!
}

/////////////////////////////////////////////////////

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
