#ifndef ASCII_SERIAL_COM_H
#define ASCII_SERIAL_COM_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "circular_buffer.h"

#define MAXMESSAGELEN 64
#define MAXDATALEN 54

/** \brief ASCII Serial Com Interface State struct
 *
 *  Keeps track of the state of the ASCII Serial Com interface
 *
 */
typedef struct ascii_serial_com_struct {
  uint8_t registerBitWidth;         /**< */
  uint8_t registerByteWidth;        /**< */
  char appVersion;                  /**< */
  char asciiSerialComVersion;       /**< */
  size_t (*fRead)(char *, size_t);  /**< */
  size_t (*fWrite)(char *, size_t); /**< */
  circular_buffer_uint8 in_buf;     /**< */
  circular_buffer_uint8 out_buf;    /**< */
  uint8_t raw_buffer[128];          /**< Raw buffer used by circular buffers */
} ascii_serial_com;

/** \brief ASCII Serial Com Interface init method
 *
 *  Initialize interface
 *
 *  \param asc is a pointer to an uninitialized ascii_serial_com struct
 *  \param registerBitWidth is the bit width of the registers
 *  \param appVersion: an ASCII byte representing the application version
 *  \param asciiSerialComVersion: an ASCII byte representing the version
 *  \param fread: the function to use to read from the serial port
 *  \param fwrite: the function to use to write to the serial port
 *
 */
// void ascii_serial_com_init(ascii_serial_com *asc, uint8_t registerBitWidth,
// char appVersion, char asciiSerialComVersion, size_t (*fRead)(char*,size_t),
// size_t (*fWrite)(char*,size_t));

/** \brief ASCII Serial Com send message
 *
 *  Send a message
 *
 *  \param asc is a pointer to an uninitialized ascii_serial_com struct
 *  \param command: a single letter command type
 *  \param data: an array of data to send
 *  \param dataLen: number of bytes of data to send
 *
 */
// void ascii_serial_com_send(ascii_serial_com *asc, char command, char* data,
// size_t dataLen);

/** \brief ASCII Serial Com receive message
 *
 *  Receive a message
 *
 *  \param asc is a pointer to an uninitialized ascii_serial_com struct
 *  \param ascVersion: the single char ASCII Serial Com version will be written
 * to this byte \param appVersion: the single char application version will be
 * written to this byte \param command: the single char command will be written
 * to this byte \param data: Data will be written to this buffer (The data
 * written can be up to MAXDATALEN bytes long, so allocate MAXDATALEN bytes)
 *  \param dataLen: The length of the data written
 *
 */
// void ascii_serial_com_receive(ascii_serial_com *asc, char* ascVerson, char*
// appVersion, char* command, char* data, size_t* dataLen);

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//////////////// Private Methods ///////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

/** \brief ASCII Serial Com Pack and put message in output buffer
 *
 *  Packs the message into the output format and push it onto the output buffer
 *
 *  \param asc is a pointer to an uninitialized ascii_serial_com struct
 *  \param command: the single char command will be written to this byte
 *  \param data: The message data
 *  \param dataLen: The length of the data
 *
 */
// void ascii_serial_com_pack_message_push_out(ascii_serial_com *asc, char
// command, char* data, size_t dataLen);

// void ascii_serial_com_pop_in_unpack(ascii_serial_com *asc, char* ascVerson,
// char* appVersion, char* command, char* data, size_t* dataLen); void
// compute_checksum(char* message, size_t* messageLen, char* outChecksum);

/** \brief convert uint8 to hex string
 *
 *  Converts uint8 to capital hex string, 2-bytes long, zero padded
 *
 *  \param num: the number to convert to hex
 *  \param outstr: a pointer to a 2-byte long string that will hold the result
 *  \param caps: hex letters are caps A-F if true, and lowercase a-f if false
 *
 */
void convert_uint8_to_hex(uint8_t num, char *outstr, bool caps);

/** \brief convert uint16 to hex string
 *
 *  Converts uint16 to capital hex string, 4-bytes long, zero padded
 *
 *  \param num: the number to convert to hex
 *  \param outstr: a pointer to a 4-byte long string that will hold the result
 *  \param caps: hex letters are caps A-F if true, and lowercase a-f if false
 *
 */
void convert_uint16_to_hex(uint16_t num, char *outstr, bool caps);

/** \brief convert uint32 to hex string
 *
 *  Converts uint32 to capital hex string, 8-bytes long, zero padded
 *
 *  \param num: the number to convert to hex
 *  \param outstr: a pointer to a 8-byte long string that will hold the result
 *  \param caps: hex letters are caps A-F if true, and lowercase a-f if false
 *
 */
void convert_uint32_to_hex(uint32_t num, char *outstr, bool caps);

/** \brief convert hex string to uint8
 *
 *  Converts hex string to uint8
 *
 *  \param str: a pointer to a 2-byte long string that holds the hex input
 *  \return the uint8_t
 *
 */
uint8_t convert_hex_to_uint8(char *instr);

/** \brief convert hex string to uint16
 *
 *  Converts hex string to uint16
 *
 *  \param str: a pointer to a 4-byte long string that holds the hex input
 *  \return the uint16_t
 *
 */
uint16_t convert_hex_to_uint16(char *instr);

/** \brief convert hex string to uint32
 *
 *  Converts hex string to uint32
 *
 *  \param str: a pointer to a 8-byte long string that holds the hex input
 *  \return the uint32_t
 *
 */
uint32_t convert_hex_to_uint32(char *instr);

#endif
