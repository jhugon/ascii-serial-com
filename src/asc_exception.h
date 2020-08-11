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

enum asc_exception {
  ASC_ERROR_UNKOWN = 0,
  ASC_ERROR_NO_ERROR = 1,
  // ASC
  ASC_ERROR_DATA_TOO_LONG = 10,
  ASC_ERROR_CHECKSUM_PROBLEM = 11, // problem computing checksum
  ASC_ERROR_INVALID_FRAME = 12,
  ASC_ERROR_INVALID_FRAME_PERIOD = 13, // relating to no '.' or misplaced '.'
  ASC_ERROR_NOT_HEX_CHAR = 19,
  // Register block
  ASC_ERROR_COMMAND_NOT_IMPLEMENTED = 20,
  ASC_ERROR_REG_BLOCK_NULL = 21,
  ASC_ERROR_UNEXPECTED_COMMAND = 22,
  ASC_ERROR_DATA_TOO_SHORT = 23,
  ASC_ERROR_REGNUM_OOB = 24, // reg number too large
  ASC_ERROR_REGVAL_LEN = 25, // reg value the wrong number of bytes
  // circular_buffer
  ASC_ERROR_CB_OOB = 50, // Circular buffer index >= size
  ASC_ERROR_CB_POP_EMPTY = 51,
  // I/O
  ASC_ERROR_FILE_READ = 90,
  ASC_ERROR_FILE_WRITE = 91,
};

#endif
