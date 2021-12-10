#ifndef ASC_EXCEPTION_H
#define ASC_EXCEPTION_H

/** \file */

//#define CEXCEPTION_T uint8_t
//#define CEXCEPTION_NONE 0xFF

#ifdef linux
#define CEXCEPTION_NO_CATCH_HANDLER(id)                                        \
  fprintf(stderr, "Uncaught exception: %" PRIu8, id);                          \
  exit(1);
#endif

#ifdef __AVR
#define CEXCEPTION_NONE (0x5A5A)
#endif

#include "externals/CException.h"

/** \brief Exception type to be used with CException
 *
 * These exception types can be used with Throw() and Catch()
 *
 */
enum asc_exception {
  ASC_ERROR_UNKOWN = 0,
  ASC_ERROR_NO_ERROR = 1,
  // ASC
  ASC_ERROR_DATA_TOO_LONG = 0x10,
  ASC_ERROR_CHECKSUM_PROBLEM = 0x11, // problem computing checksum
  ASC_ERROR_INVALID_FRAME = 0x12,
  ASC_ERROR_INVALID_FRAME_PERIOD = 0x13, // relating to no '.' or misplaced '.'
  ASC_ERROR_NOT_HEX_CHAR = 0x14,
  ASC_ERROR_COMMAND_NOT_IMPLEMENTED = 0x15,
  ASC_ERROR_UNEXPECTED_COMMAND = 0x16,
  ASC_ERROR_DATA_TOO_SHORT = 0x17,
  // Register block
  ASC_ERROR_REG_BLOCK_NULL = 0x20,
  ASC_ERROR_REGNUM_OOB = 0x21, // reg number too large
  ASC_ERROR_REGVAL_LEN = 0x22, // reg value the wrong number of bytes
  // circular_buffer
  ASC_ERROR_CB_OOB = 0x30, // Circular buffer index >= size
  ASC_ERROR_CB_POP_EMPTY = 0x31,
  ASC_ERROR_CB_BAD_CAPACITY = 0x32, // capacity not a power of 2
  // I/O
  ASC_ERROR_FILE_READ = 0x40,
  ASC_ERROR_FILE_WRITE = 0x41,
};

#endif
