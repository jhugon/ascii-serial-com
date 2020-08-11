#ifndef ASCII_SERIAL_COM_REGISTER_POINTERS_H
#define ASCII_SERIAL_COM_REGISTER_POINTERS_H

/** \file ascii_serial_com_register_pointers.h
 *
 * \brief ASCII Serial Com Register Pointers
 *
 * Register interface for devices.
 *
 * This interface contains an array of pointers, so that each message address
 * points to an arbitrary address. Additionally, a write mask controls which
 * bits of the pointed to words are writable.
 *
 * You probably want to create the array of pointers like:
 *
 * #define NREGS 6
 * REGTYPE * reg_pointers[NREGS] = {
 *  &x,
 *  &y,
 *  &z,
 *  NULL,
 *  NULL,
 *  NULL
 * };
 *
 * REGTYPE reg_write_masks[NREGS] = {
 *  0,
 *  0x3,
 *  0xFF,
 *  0,
 *  0,
 *  0
 * }
 *
 *
 * ascii_serial_com_register_pointers_init(&rps,reg_pointers,reg_write_masks,NREGS);
 *
 */

#include "ascii_serial_com.h"

#define REGWIDTHBITS 8
#define REGWIDTHBYTES 1
#define REGTYPE uint8_t
#define REGPRINTTYPEINT PRIu8
#define REGPRINTTYPEHEX PRIX8

/** \brief ASCII Serial Com Register Pointers State struct
 *
 *  Keeps track of the state of the ASCII Serial Com Register Pointers
 *
 */
typedef struct ascii_serial_com_register_pointers_struct {
  REGTYPE *
      *pointers; /**< points to start of block of register pointers of memory */
  REGTYPE *write_masks; /**< points to start of block write masks */
  uint16_t n_regs; /**< number of registers (number of registers not necessarily
                      number of bytes) */
} ascii_serial_com_register_pointers;

/** \brief ASCII Serial Com Register Pointers init
 *
 * Initialize ASCII Serial Com register_pointers
 *
 * \param register_pointers_state should be an uninitialized
 * ascii_serial_com_register_pointers object
 *
 * \param pointers points to an array of pointers to registers (entries may be
 * NULL)
 *
 * \param an array of write masks. Every one bit in these masks is a bit that
 * may be written to the registers.
 *
 * \param n_regs is the number of registers in the pointers (not necessarily the
 * number of bytes)
 *
 */
void ascii_serial_com_register_pointers_init(
    ascii_serial_com_register_pointers *register_pointers_state,
    REGTYPE **pointers, REGTYPE *write_masks, uint16_t n_regs);

/** \brief ASCII Serial Com Register Pointers handle message
 *
 * This is the function passed to ascii_serial_com_device as frw
 *
 * The parameters are the same as in that function (and
 * ascii_serial_com_get_message_from_input_buffer + register_pointers_state).
 *
 * WILL CLOBBER data
 *
 * \param register_pointers_state should be a pointer to an initialized
 * ascii_serial_com_register_pointers
 *
 */
void ascii_serial_com_register_pointers_handle_message(
    ascii_serial_com *asc, char ascVersion, char appVersion, char command,
    char *data, size_t dataLen, void *register_pointers_state);

#endif
