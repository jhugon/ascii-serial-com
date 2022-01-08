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
 * ascii_serial_com_register_pointers_init(&rps,reg_pointers,reg_write_masks,NREGS);
 *
 */

#include "ascii_serial_com.h"

#ifdef __AVR
#define REGWIDTHBITS 8
#define REGWIDTHBYTES 1
#define REGTYPE uint8_t
#define REGPRINTTYPEINT PRIu8
#define REGPRINTTYPEHEX PRIX8
#else
#define REGWIDTHBITS 32
#define REGWIDTHBYTES 4
#define REGTYPE uint32_t
#define REGPRINTTYPEINT PRIu32
#define REGPRINTTYPEHEX PRIX32
#endif

/** \brief ASCII Serial Com Register Pointers State struct
 *
 *  Keeps track of the state of the ASCII Serial Com Register Pointers
 *
 */
typedef struct ascii_serial_com_register_pointers_struct {
  volatile REGTYPE *
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
 * NULL). It's volatile so that it can point to device registers without
 * reads/writes to them being optimized out.
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
    volatile REGTYPE **pointers, REGTYPE *write_masks, uint16_t n_regs);

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

///////////////////
// Helper macros //
///////////////////

#define _extraInputBuffer_size_ 64

// Don't use a semicolon after this?
#define DECLARE_ASC_DEVICE_W_REGISTER_POINTERS()                               \
  void handle_nf_messages(ascii_serial_com *asc, char ascVersion,              \
                          char appVersion, char command, char *data,           \
                          size_t dataLen, void *state_vp);                     \
  bool streaming_is_on = false;                                                \
  uint8_t tmp_byte;                                                            \
  uint8_t extraInputBuffer_raw[_extraInputBuffer_size_];                       \
  circular_buffer_uint8 extraInputBuffer;                                      \
  ascii_serial_com_device ascd;                                                \
  ascii_serial_com_register_pointers reg_pointers_state;                       \
  ascii_serial_com_device_config ascd_config = {                               \
      .func_rw = ascii_serial_com_register_pointers_handle_message,            \
      .state_rw = &reg_pointers_state,                                         \
      .func_nf = handle_nf_messages,                                           \
      .state_nf = &streaming_is_on};                                           \
  circular_buffer_uint8 *asc_in_buf;                                           \
  circular_buffer_uint8 *asc_out_buf;                                          \
  void handle_nf_messages(__attribute__((unused)) ascii_serial_com *asc,       \
                          __attribute__((unused)) char ascVersion,             \
                          __attribute__((unused)) char appVersion,             \
                          char command, __attribute__((unused)) char *data,    \
                          __attribute__((unused)) size_t dataLen,              \
                          void *state_vp) {                                    \
    bool *state = (bool *)state_vp;                                            \
    if (command == 'n') {                                                      \
      *state = true;                                                           \
    } else if (command == 'f') {                                               \
      *state = false;                                                          \
    }                                                                          \
  }

// Make sure to put inside Try block
// Use a semicolon on this one
#define SETUP_ASC_DEVICE_W_REGISTER_POINTERS(register_map,                     \
                                             register_write_masks, nRegs)      \
  ascii_serial_com_register_pointers_init(&reg_pointers_state, register_map,   \
                                          register_write_masks, nRegs);        \
  ascii_serial_com_device_init(&ascd, &ascd_config);                           \
  asc_in_buf = ascii_serial_com_device_get_input_buffer(&ascd);                \
  asc_out_buf = ascii_serial_com_device_get_output_buffer(&ascd);              \
                                                                               \
  circular_buffer_init_uint8(&extraInputBuffer, _extraInputBuffer_size_,       \
                             extraInputBuffer_raw)

// Make sure is in a Try block and put a semicolon after this one
// Write data to usart from output buffer
// Read data from extra input buffer into input buffer
// Parse and handle received messages
//
// Assumes this is STM32 and that something else rx bytes and puts them in
// extraInputBuffer
#define HANDLE_ASC_COMM_IN_POLLING_LOOP(usart)                                 \
  if (!circular_buffer_is_empty_uint8(asc_out_buf) &&                          \
      (USART_ISR(usart) & USART_ISR_TXE)) {                                    \
    tmp_byte = circular_buffer_pop_front_uint8(asc_out_buf);                   \
    usart_send(usart, tmp_byte);                                               \
  }                                                                            \
  if (!circular_buffer_is_empty_uint8(&extraInputBuffer)) {                    \
    CM_ATOMIC_BLOCK() {                                                        \
      tmp_byte = circular_buffer_pop_front_uint8(&extraInputBuffer);           \
    }                                                                          \
    circular_buffer_push_back_uint8(asc_in_buf, tmp_byte);                     \
  }                                                                            \
  ascii_serial_com_device_receive(&ascd)

#endif
