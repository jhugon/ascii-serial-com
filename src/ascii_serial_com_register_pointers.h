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
 *      #define NREGS 6
 *      REGTYPE * reg_pointers[NREGS] = {
 *       &x,
 *       &y,
 *       &z,
 *       NULL,
 *       NULL,
 *       NULL
 *      };
 *
 *      REGTYPE reg_write_masks[NREGS] = {
 *       0,
 *       0x3,
 *       0xFF,
 *       0,
 *       0,
 *       0
 *      }
 *
 *      ascii_serial_com_register_pointers_init(&rps,reg_pointers,reg_write_masks,NREGS);
 *
 * ## Convenience Macros
 *
 * A set of convenience macros are provided to ease the common use of
 * ascii_serial_com_register_pointers with ascii_serial_com_device.
 *
 * ### Required Macros
 *
 * To use the macros:
 *
 * 1. Declare the register pointer and register write mask array as above
 * 2. Before the main() function, put `DECLARE_ASC_DEVICE_W_REGISTER_POINTERS()`
 * on a line **with no semicolon**
 * 3. In the setup portion of main(), before the polling loop, inside a
 * Try/Catch block, put:
 * `SETUP_ASC_DEVICE_W_REGISTER_POINTERS(reg_map,reg_write_masks, NREGS);`,
 * where the variables are declared as in the previous section.
 * 4. Inside the polling loop and a Try/Catch block, put:
 * `HANDLE_ASC_COMM_IN_POLLING_LOOP(USART1);`, replacing USART1 with the
 * appropriate USART.
 *
 * Number 4 assumes something is putting received bytes into a
 * circular_buffer_uint8 called `extraInputBuffer` (declared and setup by the
 * macros). This can be accomplished using, for STM32,
 * `def_usart_isr_push_rx_to_circ_buf(<usart_isr>,<usart>,&extraInputBuffer)`
 * (no semicolon)
 *
 * ## Streaming
 *
 * The boolean valued macro `READY_TO_STREAM_ASC_DEVICE_W_REGISTER_POINTERS`
 * should be checked before actually streaming a message. and
 * `STREAM_TO_HOST_ASC_DEVICE_W_REGISTER_POINTERS(data,data_len);` should be
 * run to send the message.
 *
 * The macros define a `bool streaming_is_on` that is initialized to false, set
 * to true when a "n" message is received, and set to false when a "f" message
 * is received. It should be used to decide when to start auxiliary routines
 * that prepare to stream messages.
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

/** \brief Declarations for ascii_serial_com_device and
 * ascii_serial_com_register_pointers
 *
 * Declarations that should be outside of (and before) main() to ease use of
 * ascii_serial_com_register_pointers with ascii_serial_com_device.
 *
 * ## Notes
 *
 * **Don't follow this with a semicolon**
 *
 * ## Parameters
 *
 * None
 *
 */
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

/** \brief Setup for ascii_serial_com_device and
 * ascii_serial_com_register_pointers
 *
 * Setup that should be inside main() before the polling loop. This is part of
 * a group of macros to ease use of ascii_serial_com_register_pointers with
 * ascii_serial_com_device.
 *
 * ## Notes
 *
 * **Make sure this is inside a Try/Catch block**
 *
 * **Follow this with a semicolon**
 *
 * ## Parameters
 *
 * register_map: the register map, type: REGTYPE * array
 *
 * register_write_masks: the register write masks, type: REGTYPE array
 *
 * nRegs: the length of the two arrays above
 *
 */
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

#if defined(__ARM_ARCH)
#define _serial_tx_buf_is_empty(usart) ((USART_ISR(usart) & USART_ISR_TXE))
#elif defined(__AVR)
#define _serial_tx_buf_is_empty(usart) (UCSR0A & (1 << UDRE0))
#else
#define _serial_tx_buf_is_empty(usart) __builtin_unreachable()
#endif

#if defined(__ARM_ARCH)
// Already defined by libopencm3
#elif defined(__AVR)
#define usart_send(reg, tmp_byte) reg = tmp_byte
#else
#define usart_send(usart, tmp_byte) __builtin_unreachable()
#endif

#if defined(__ARM_ARCH)
#define _ATOMIC CM_ATOMIC_BLOCK()
#elif defined(__AVR)
#define _ATOMIC ATOMIC_BLOCK(ATOMIC_FORCEON)
#else
#define _ATOMIC __builtin_unreachable()
#endif

/** \brief Polling for ascii_serial_com_device and
 * ascii_serial_com_register_pointers
 *
 * Within the polling loop, handles transmitting bytes, processing received
 * bytes, and handling received messages. This is part of a group of macros to
 * ease use of ascii_serial_com_register_pointers with ascii_serial_com_device.
 *
 * ## Notes
 *
 * **Assumes that something else receives bytes and puts them in**
 * `extraInputBuffer`
 *
 * **Make sure this is inside a Try/Catch block**
 *
 * **Follow this with a semicolon**
 *
 * ## Parameters
 *
 * usart: the address of the USART used for transmitting bytes. For STM32:
 * USART1, USART2, .... For AVR: UDR0, ....
 *
 */
#define HANDLE_ASC_COMM_IN_POLLING_LOOP(usart)                                 \
  if (!circular_buffer_is_empty_uint8(asc_out_buf) &&                          \
      _serial_tx_buf_is_empty(usart)) {                                        \
    tmp_byte = circular_buffer_pop_front_uint8(asc_out_buf);                   \
    usart_send(usart, tmp_byte);                                               \
  }                                                                            \
  if (!circular_buffer_is_empty_uint8(&extraInputBuffer)) {                    \
    _ATOMIC { tmp_byte = circular_buffer_pop_front_uint8(&extraInputBuffer); } \
    circular_buffer_push_back_uint8(asc_in_buf, tmp_byte);                     \
  }                                                                            \
  ascii_serial_com_device_receive(&ascd)

/** \brief Check if you should stream a message to the host
 *
 * This is a boolean value macro that checks if streaming is enabled and the
 * transmit buffer is empty. If true, the user may stream a message to the host
 * now with STREAM_TO_HOST_ASC_DEVICE_W_REGISTER_POINTERS
 *
 * This is part of a group of macros to ease use of
 * ascii_serial_com_register_pointers with ascii_serial_com_device.
 *
 * ## Notes
 *
 * **Make sure this is inside a Try/Catch block**
 *
 */
#define READY_TO_STREAM_ASC_DEVICE_W_REGISTER_POINTERS                         \
  (streaming_is_on && circular_buffer_get_size_uint8(asc_out_buf) == 0)

/** \brief Stream data to the host
 *
 * This macro puts a message in the output buffer
 *
 * This is part of a group of macros to ease use of
 * ascii_serial_com_register_pointers with ascii_serial_com_device.
 *
 * ## Notes
 *
 * **Make sure this is inside a Try/Catch block**
 *
 * ## Parameters
 *
 * data: a pointer to data or array of data to be sent
 *
 * data_size: a uint describing the data size in bytes
 *
 */
#define STREAM_TO_HOST_ASC_DEVICE_W_REGISTER_POINTERS(data, data_size)         \
  ascii_serial_com_device_put_s_message_in_output_buffer(&ascd, '0', '0',      \
                                                         data, data_size)

#endif
