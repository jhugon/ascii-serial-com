#ifndef ASCII_SERIAL_COM_DEVICE_H
#define ASCII_SERIAL_COM_DEVICE_H

/** \brief ASCII Serial Com Device
 *
 */

#include "ascii_serial_com.h"

/** \brief ASCII Serial Com Device State struct
 *
 *  Keeps track of the state of the ASCII Serial Com device
 *
 * The functions take the stuff from
 * ascii_serial_com_get_message_from_input_buffer plus a void pointer to
 * possible state/configuration info. If the functions are null ptrs, an error
 * message returned to host. This class owns the data buffer passed around.
 */
typedef struct ascii_serial_com_device_struct {
  ascii_serial_com asc;        /**< used to receive messages and reply */
  char[MAXDATALEN] dataBuffer; /**< data part of message received here */
  void (*frw)(ascii_serial_com *, char, char, char, const char *data, size_t,
              void *); /**< called for r or w messages */
  void (*fs)(ascii_serial_com *, char, char, char, const char *data, size_t,
             void *); /**< called for s messages */
  void (*fother)(ascii_serial_com *, char, char, char, const char *data, size_t,
                 void *); /**< called for other messages */
} ascii_serial_com_device;

/** \brief ASCII Serial Com Device init
 *
 * Initialize ASCII Serial Com device
 *
 * The functions take the stuff from
 * ascii_serial_com_get_message_from_input_buffer plus a void pointer to
 * possible state/configuration info. If the functions are null ptrs, an error
 * message returned to host. This class owns the data buffer passed around.
 *
 */
void ascii_serial_com_device_init(
    void (*frw)(ascii_serial_com *, char, char, char, const char *data, size_t,
                void *); /**< called for r or w messages */
    void (*fs)(ascii_serial_com *, char, char, char, const char *data, size_t,
               void *); /**< called for s messages */
    void (*fother)(ascii_serial_com *, char, char, char, const char *data,
                   size_t, void *); /**< called for other messages */
);

/** \brief ASCII Serial Com Device receive messages
 *
 * Initialize ASCII Serial Com device. Actually does the work.
 *
 * Receives ASCII Serial Com messages, and hands them over to the
 * appropriate one of the functions
 *
 */
void ascii_serial_com_device_receive();

#endif
