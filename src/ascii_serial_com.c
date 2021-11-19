#include "ascii_serial_com.h"

#define crc_bbb 0
#define crc_bbf 1
#define crc_tbl4bit 2
#define crc_crcmod 3
#define crc_option crc_tbl4bit

#if crc_option == crc_bbb
#include "crc_16_dnp_bbb.h"
#define crc_init crc_16_dnp_bbb_init
#define crc_update crc_16_dnp_bbb_update
#define crc_finalize crc_16_dnp_bbb_finalize
#elif crc_option == crc_bbf
#include "crc_16_dnp_bbf.h"
#define crc_init crc_16_dnp_bbf_init
#define crc_update crc_16_dnp_bbf_update
#define crc_finalize crc_16_dnp_bbf_finalize
#elif crc_option == crc_tbl4bit
#include "crc_16_dnp_tbl4bit.h"
#define crc_init crc_16_dnp_tbl4bit_init
#define crc_update crc_16_dnp_tbl4bit_update
#define crc_finalize crc_16_dnp_tbl4bit_finalize
#elif crc_option == crc_crcmod
#include "crc_16_dnp_crcmod.h"
#else
#error "crc_option not supported!"
#endif

/** \file */

#include <inttypes.h>
//#include <stdio.h>

#define error_message_data_len 12
#define max_remove_unfinished_frames_tries 10

void ascii_serial_com_init(ascii_serial_com *asc) {

  circular_buffer_init_uint8(&(asc->in_buf), MAXMESSAGELEN, asc->raw_buffer);
  circular_buffer_init_uint8(&(asc->out_buf), MAXMESSAGELEN,
                             asc->raw_buffer + MAXMESSAGELEN);

  asc->send_stream_frame_counter = 0;
  asc->receive_stream_frame_counter = 0;
  asc->receive_stream_frame_counter_initialized = false;
  asc->ignoreCRCMismatch = false;
}

void ascii_serial_com_put_message_in_output_buffer(
    ascii_serial_com *asc, char ascVersion, char appVersion, char command,
    const char *data, size_t dataLen) {
  // fprintf(stderr,"ascii_serial_com_put_message_in_output_buffer called with
  // ascVer: %c appVer: %c command: %c datalen: %zu\n",ascVersion, appVersion,
  // command, dataLen); fprintf(stderr,"in_buf: %p\n",asc->in_buf.buffer);
  // fprintf(stderr,"out_buf: %p\n",asc->out_buf.buffer);

  if (dataLen > MAXDATALEN) {
    Throw(ASC_ERROR_DATA_TOO_LONG);
  }
  circular_buffer_push_back_uint8(&asc->out_buf, '>');
  circular_buffer_push_back_uint8(&asc->out_buf, ascVersion);
  circular_buffer_push_back_uint8(&asc->out_buf, appVersion);
  circular_buffer_push_back_uint8(&asc->out_buf, command);
  for (size_t i = 0; i < dataLen; i++) {
    circular_buffer_push_back_uint8(&asc->out_buf, data[i]);
  }
  circular_buffer_push_back_uint8(&asc->out_buf, '.');
  char checksum[NCHARCHECKSUM];
  ascii_serial_com_compute_checksum(asc, checksum, true);
  for (size_t i = 0; i < NCHARCHECKSUM; i++) {
    circular_buffer_push_back_uint8(&asc->out_buf, checksum[i]);
  }
  circular_buffer_push_back_uint8(&asc->out_buf, '\n');
}

void ascii_serial_com_get_message_from_input_buffer(ascii_serial_com *asc,
                                                    char *ascVersion,
                                                    char *appVersion,
                                                    char *command, char *data,
                                                    size_t *dataLen) {
  char computeChecksum[NCHARCHECKSUM];
  size_t buf_size = circular_buffer_get_size_uint8(&asc->in_buf);
  if (buf_size == 0) {
    //    fprintf(stderr, "Error Buffer size == 0\n");
    *command = '\0';
    *dataLen = 0;
    return;
  }
  circular_buffer_remove_front_unfinished_frames_uint8(&asc->in_buf, '>', '\n');
  buf_size = circular_buffer_get_size_uint8(&asc->in_buf); // could have changed
  size_t iEnd = circular_buffer_find_first_uint8(&asc->in_buf, '\n');
  if (iEnd >= buf_size) {
    //    fprintf(stderr, "Error: start and/or end of frame not found\n");
    // Don't throw away frame, the rest of the frame may just nead another read
    // to get
    *command = '\0';
    *dataLen = 0;
    return;
  }
  size_t iPeriod = circular_buffer_find_first_uint8(&asc->in_buf, '.');
  if (iPeriod > iEnd) {
    for (size_t i = 0; i <= iEnd; i++) {
      circular_buffer_pop_front_uint8(&asc->in_buf); // pop off bad frame
    }
    Throw(ASC_ERROR_INVALID_FRAME_PERIOD);
  }
  if (!asc->ignoreCRCMismatch) {
    ascii_serial_com_compute_checksum(asc, computeChecksum, false);
  }
  // fprintf(stderr,"Received message: \n");
  // circular_buffer_print_uint8(&asc->in_buf,stderr);
  circular_buffer_pop_front_uint8(&asc->in_buf); // pop off starting '>'
  *ascVersion = circular_buffer_pop_front_uint8(&asc->in_buf);
  *appVersion = circular_buffer_pop_front_uint8(&asc->in_buf);
  *command = circular_buffer_pop_front_uint8(&asc->in_buf);
  *dataLen = 0;
  for (size_t iData = 0; iData < iPeriod - 4; iData++) {
    char dataByte = circular_buffer_pop_front_uint8(&asc->in_buf);
    if (dataByte == '.') {
      break;
    }
    data[iData] = dataByte;
    *dataLen += 1;
  }
  circular_buffer_pop_front_uint8(&asc->in_buf); // pop off '.'
  if (circular_buffer_get_size_uint8(&asc->in_buf) < NCHARCHECKSUM + 1 ||
      circular_buffer_get_element_uint8(&asc->in_buf, NCHARCHECKSUM) != '\n') {
    Throw(ASC_ERROR_INVALID_FRAME);
    // fprintf(stderr, "Error: checksum incorrect length\n");
    //*command = '\0';
    //*dataLen = 0;
    // return;
  }
  char receiveChecksum[NCHARCHECKSUM];
  for (size_t iChk = 0; iChk < NCHARCHECKSUM; iChk++) {
    receiveChecksum[iChk] = circular_buffer_pop_front_uint8(&asc->in_buf);
  }
  if (!asc->ignoreCRCMismatch) {
    for (size_t iChk = 0; iChk < NCHARCHECKSUM; iChk++) {
      if (receiveChecksum[iChk] != computeChecksum[iChk]) {
        // checksum mismatch!
        //      fprintf(stderr, "Error: checksum mismatch, computed: ");
        //      for (size_t i = 0; i < NCHARCHECKSUM; i++) {
        //        fprintf(stderr, "%c", computeChecksum[i]);
        //      }
        //      fprintf(stderr, ", received: ");
        //      for (size_t i = 0; i < NCHARCHECKSUM; i++) {
        //        fprintf(stderr, "%c", receiveChecksum[i]);
        //      }
        //      fprintf(stderr, "\n");
        circular_buffer_pop_front_uint8(&asc->in_buf); // pop off remaining '\n'
        *command = '\0';
        *dataLen = 0;
        return;
      }
    }
  }
  circular_buffer_pop_front_uint8(&asc->in_buf); // pop off trailing '\n'
  return;                                        // success!
}

circular_buffer_uint8 *
ascii_serial_com_get_input_buffer(ascii_serial_com *asc) {
  return &asc->in_buf;
}

circular_buffer_uint8 *
ascii_serial_com_get_output_buffer(ascii_serial_com *asc) {
  return &asc->out_buf;
}

void ascii_serial_com_compute_checksum(ascii_serial_com *asc, char *checksumOut,
                                       bool outputBuffer) {
  circular_buffer_uint8 *circ_buf;
  if (outputBuffer) {
    circ_buf = &asc->out_buf;
  } else {
    circ_buf = &asc->in_buf;
  }
  const size_t size = circular_buffer_get_size_uint8(circ_buf);
  size_t iStart = circular_buffer_find_last_uint8(circ_buf, '>');
  size_t iStop = circular_buffer_find_last_uint8(circ_buf, '.');
  if (!outputBuffer) { // want the first one on input
    iStart = circular_buffer_find_first_uint8(circ_buf, '>');
    iStop = circular_buffer_find_first_uint8(circ_buf, '.');
  }
  if (iStart >= size) {
    Throw(ASC_ERROR_INVALID_FRAME);
  }
  if (iStop >= size) {
    Throw(ASC_ERROR_INVALID_FRAME_PERIOD);
  }
  if (iStop >= MAXMESSAGELEN - NCHARCHECKSUM - 1) {
    Throw(ASC_ERROR_CHECKSUM_PROBLEM);
  }
  if (iStop <= iStart || iStop - iStart < 4) {
    Throw(ASC_ERROR_INVALID_FRAME_PERIOD);
  }
  uint8_t *blocks[2];
  size_t blocks_sizes[2];
  size_t nBlocks = circular_buffer_get_blocks_uint8(
      circ_buf, iStart, iStop - iStart + 1, blocks, blocks_sizes);
#if crc_option == crc_crcmod
  uint16_t crc = 0xFFFF;
  for (size_t iBlock = 0; iBlock < nBlocks; iBlock++) {
    crc = computeCRC_16_DNP(blocks[iBlock], blocks_sizes[iBlock], crc);
  }
#else
  uint16_t crc = crc_init();
  for (size_t iBlock = 0; iBlock < nBlocks; iBlock++) {
    crc = crc_update(crc, blocks[iBlock], blocks_sizes[iBlock]);
  }
  crc = crc_finalize(crc);
#endif
  convert_uint16_to_hex(crc, checksumOut, true);
}

void ascii_serial_com_put_s_message_in_output_buffer(ascii_serial_com *asc,
                                                     char ascVersion,
                                                     char appVersion,
                                                     const char *data,
                                                     size_t dataLen) {

  if (dataLen > MAXSPAYLOADEN) {
    Throw(ASC_ERROR_DATA_TOO_LONG);
  }
  char outData[MAXDATALEN];
  convert_uint8_to_hex(asc->send_stream_frame_counter, outData, true);
  asc->send_stream_frame_counter++;
  outData[2] = ',';
  for (size_t i = 0; i < dataLen && i < (MAXDATALEN - 3); i++) {
    outData[i + 3] = data[i];
  }
  ascii_serial_com_put_message_in_output_buffer(asc, ascVersion, appVersion,
                                                's', outData, dataLen + 3);
}

void ascii_serial_com_put_error_in_output_buffer(ascii_serial_com *asc,
                                                 char ascVersion,
                                                 char appVersion, char command,
                                                 char *data, size_t dataLen,
                                                 enum asc_exception errorCode) {
  char outData[error_message_data_len];
  convert_uint8_to_hex((uint8_t)errorCode, outData, true);
  outData[2] = command;
  for (size_t i = 0; i < dataLen && i < (error_message_data_len - 3); i++) {
    outData[i + 3] = data[i];
  }
  size_t outDataLen = dataLen + 3;
  if (outDataLen > error_message_data_len) {
    outDataLen = error_message_data_len;
  }
  ascii_serial_com_put_message_in_output_buffer(asc, ascVersion, appVersion,
                                                'e', outData, outDataLen);
}

void ascii_serial_com_set_ignore_CRC_mismatch(ascii_serial_com *asc) {
  asc->ignoreCRCMismatch = true;
}

void ascii_serial_com_unset_ignore_CRC_mismatch(ascii_serial_com *asc) {
  asc->ignoreCRCMismatch = false;
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

uint8_t convert_hex_to_uint8(const char *instr) {
  uint8_t result = 0;
  for (uint8_t i = 0; i < 2; i++) {
    char thischar = instr[1 - i];
    if (thischar >= 0x30 && thischar <= 0x39) {
      result |= (thischar - 0x30) << (i * 4);
    } else if (thischar >= 0x41 && thischar <= 0x46) {
      result |= (thischar - 0x41 + 10) << (i * 4);
    } else if (thischar >= 0x61 && thischar <= 0x66) {
      result |= (thischar - 0x61 + 10) << (i * 4);
    } else {
      //      fprintf(stderr,"Problem char: '%c' =
      //      %"PRIX8"\n",thischar,(uint8_t) thischar); fflush(stderr);
      Throw(ASC_ERROR_NOT_HEX_CHAR);
    }
  }
  return result;
}

uint16_t convert_hex_to_uint16(const char *instr) {
  uint16_t result = 0;
  for (uint8_t i = 0; i < 2; i++) {
    result |= (uint16_t)convert_hex_to_uint8(instr + (1 - i) * 2) << (8 * i);
  }
  return result;
}

uint32_t convert_hex_to_uint32(const char *instr) {
  uint32_t result = 0;
  for (uint8_t i = 0; i < 4; i++) {
    result |= (uint32_t)convert_hex_to_uint8(instr + (3 - i) * 2) << (8 * i);
  }
  return result;
}
