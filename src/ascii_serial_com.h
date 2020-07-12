#ifndef ASCII_SERIAL_COM_H
#define ASCII_SERIAL_COM_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "circular_buffer.h"

#define MAXMESSAGELEN 64
#define MAXDATALEN 54
#define NCHARCHECKSUM 4

/** \brief ASCII Serial Com Interface State struct
 *
 *  Keeps track of the state of the ASCII Serial Com interface
 *
 */
typedef struct ascii_serial_com_struct {
  size_t (*fRead)(char *, size_t);        /**< */
  size_t (*fWrite)(const char *, size_t); /**< */
  circular_buffer_uint8 in_buf;           /**< */
  circular_buffer_uint8 out_buf;          /**< */
  uint8_t
      raw_buffer[2 * MAXMESSAGELEN]; /**< Raw buffer used by circular buffers */
} ascii_serial_com;

/** \brief ASCII Serial Com Interface init method
 *
 *  Initialize interface
 *
 *  \param asc is a pointer to an uninitialized ascii_serial_com struct
 *
 *  \param fread: the function to use to read from the serial port
 *
 *  \param fwrite: the function to use to write to the serial port
 *
 */
void ascii_serial_com_init(ascii_serial_com *asc,
                           size_t (*fRead)(char *, size_t),
                           size_t (*fWrite)(const char *, size_t));

/** \brief ASCII Serial Com send message
 *
 *  Send a message
 *
 *  \param asc is a pointer to an initialized ascii_serial_com struct
 *
 *  \param ascVersion: the single char ASCII Serial Com version (probably '0')
 *
 *  \param appVersion: the single char application version (user application
 * info)
 *
 *  \param command: a single letter command type
 *
 *  \param data: an array of data to send
 *
 *  \param dataLen: number of bytes of data to send
 *
 */
void ascii_serial_com_send(ascii_serial_com *asc, char ascVersion,
                           char appVersion, char command, char *data,
                           size_t dataLen);

/** \brief ASCII Serial Com receive message
 *
 *  Receive a message
 *
 *  \param asc is a pointer to an initialized ascii_serial_com struct
 *
 *  \param ascVersion: the single char ASCII Serial Com version will be written
 *  to this byte
 *
 *  \param appVersion: the single char application version will be
 *  written to this byte
 *
 *  \param command: the single char command will be written
 *  to this byte. Will be '\0' if no message in read
 *
 *  \param data: Data will be written to this buffer (The data
 *  written can be up to MAXDATALEN bytes long, so allocate MAXDATALEN bytes)
 *
 *  \param dataLen: The length of the data astually written
 *
 */
void ascii_serial_com_receive(ascii_serial_com *asc, char *ascVersion,
                              char *appVersion, char *command, char *data,
                              size_t *dataLen);

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//////////////// Private Methods ///////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

/** \brief ASCII Serial Com Pack and put message in output buffer
 *
 *  Packs the message into the output format and push it onto the output buffer
 *  USER'S RESPONSIBILITY TO MAKE SURE MESSAGE CAN FIT IN OUTPUT CIRCULAR
 * BUFFER. Message length is dataLen + (MAXMESSAGELEN-MAXMESSAGELEN)
 *
 *  \param asc is a pointer to an initialized ascii_serial_com struct
 *
 *  \param ascVersion: the single char ASCII Serial Com version (probably '0')
 *
 *  \param appVersion: the single char application version (user application
 * info)
 *
 *  \param command: the single char command will be written to this byte
 *
 *  \param data: The message data
 *
 *  \param dataLen: The length of the data
 */
void ascii_serial_com_pack_message_push_out(ascii_serial_com *asc,
                                            char ascVersion, char appVersion,
                                            char command, char *data,
                                            size_t dataLen);

/** \brief ASCII Serial Com pop message from input buffer and unpack
 *
 *  Unpacks the first message found in the input buffer
 *
 *  \param asc is a pointer to an initialized ascii_serial_com struct
 *
 *  All other parameters are outputs. Command will be set to \0 if no message
 * found in buffer
 *
 * \param ascVersion: the single char ASCII-Serial-Com version
 * in the message will be written here
 *
 * \param appVersion: the single char application version in the message will be
 * written here
 *
 * \param command: the single char command will be written to this byte
 *
 * \param data: The message data will be put here. Should point to a MAXDATALEN
 * long buffer
 *
 * \param dataLen: The length of the data put in data
 */
void ascii_serial_com_pop_in_unpack(ascii_serial_com *asc, char *ascVersion,
                                    char *appVersion, char *command, char *data,
                                    size_t *dataLen);

/** \brief ASCII Serial Com compute checksum of message
 *
 *
 *  Computes the checksum of the last message in the input or output buffer.
 * Specifically finds the last substring starting with the last '>' and ending
 * in the last '.'.
 *
 *  \param asc is a pointer to an initialized ascii_serial_com struct
 *
 *  \param checksumOut: pointer to already initialized buffer NCHARCHECKSUM long
 *
 *  \param outputBuffer: if true, use output buffer, if false use input buffer
 *
 *  \return true if checksum valid
 *
 */
bool ascii_serial_com_compute_checksum(ascii_serial_com *asc, char *checksumOut,
                                       bool outputBuffer);

/** \brief convert uint8 to hex string
 *
 *  Converts uint8 to capital hex string, 2-bytes long, zero padded
 *
 *  \param num: the number to convert to hex
 *
 *  \param outstr: a pointer to a 2-byte long string that will hold the result
 *
 *  \param caps: hex letters are caps A-F if true, and lowercase a-f if false
 *
 */
void convert_uint8_to_hex(uint8_t num, char *outstr, bool caps);

/** \brief convert uint16 to hex string
 *
 *  Converts uint16 to capital hex string, 4-bytes long, zero padded
 *
 *  \param num: the number to convert to hex
 *
 *  \param outstr: a pointer to a 4-byte long string that will hold the result
 *
 *  \param caps: hex letters are caps A-F if true, and lowercase a-f if false
 *
 */
void convert_uint16_to_hex(uint16_t num, char *outstr, bool caps);

/** \brief convert uint32 to hex string
 *
 *  Converts uint32 to capital hex string, 8-bytes long, zero padded
 *
 *  \param num: the number to convert to hex
 *
 *  \param outstr: a pointer to a 8-byte long string that will hold the result
 *
 *  \param caps: hex letters are caps A-F if true, and lowercase a-f if false
 *
 */
void convert_uint32_to_hex(uint32_t num, char *outstr, bool caps);

/** \brief convert hex string to uint8
 *
 *  Converts hex string to uint8
 *
 *  \param str: a pointer to a 2-byte long string that holds the hex input
 *
 *  \return the uint8_t
 *
 */
uint8_t convert_hex_to_uint8(char *instr);

/** \brief convert hex string to uint16
 *
 *  Converts hex string to uint16
 *
 *  \param str: a pointer to a 4-byte long string that holds the hex input
 *
 *  \return the uint16_t
 *
 */
uint16_t convert_hex_to_uint16(char *instr);

/** \brief convert hex string to uint32
 *
 *  Converts hex string to uint32
 *
 *  \param str: a pointer to a 8-byte long string that holds the hex input
 *
 *  \return the uint32_t
 *
 */
uint32_t convert_hex_to_uint32(char *instr);

#endif
