#ifndef ASCII_SERIAL_COM_REGISTER_BLOCK_H
#define ASCII_SERIAL_COM_REGISTER_BLOCK_H

/** \brief ASCII Serial Com Register Block
 *
 * Register interface for devices.
 *
 * This interface assumes the registers are a contigous block of memory
 *
 */

#include "ascii_serial_com.h"

#define REGWIDTHBITS 8
#define REGWIDTHBYTES 1
#define REGTYPE uint8_t
#define REGPRINTTYPEINT PRIu8
#define REGPRINTTYPEHEX PRIX8

/** \brief ASCII Serial Com Register Block State struct
 *
 *  Keeps track of the state of the ASCII Serial Com Register Block
 *
 */
typedef struct ascii_serial_com_register_block_struct {
  REGTYPE *block;  /**< points to start of register block of memory */
  uint16_t n_regs; /**< size of block (number of registers not necessarily
                      number of bytes) */
} ascii_serial_com_register_block;

/** \brief ASCII Serial Com Register Block init
 *
 * Initialize ASCII Serial Com register_block
 *
 * \param register_block_state should be an uninitialized
 * ascii_serial_com_register_block object
 *
 * \param block points to start of register block of memory
 *
 * \param n_regs is the number of registers in the block (not necessarily the
 * number of bytes)
 *
 */
void ascii_serial_com_register_block_init(
    ascii_serial_com_register_block *register_block_state, REGTYPE *block,
    uint16_t n_regs);

/** \brief ASCII Serial Com Register Block handle message
 *
 * This is the function passed to ascii_serial_com_device as frw
 *
 * The parameters are the same as in that function (and
 * ascii_serial_com_get_message_from_input_buffer + register_block_state).
 *
 * WILL CLOBBER data
 *
 * \param register_block_state should be a pointer to an initialized
 * ascii_serial_com_register_block
 *
 */
void ascii_serial_com_register_block_handle_message(
    ascii_serial_com *asc, char ascVersion, char appVersion, char command,
    char *data, size_t dataLen, void *register_block_state);

#endif
