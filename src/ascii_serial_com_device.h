#ifndef ASCII_SERIAL_COM_DEVICE_H
#define ASCII_SERIAL_COM_DEVICE_H

/** \file ascii_serial_com_device.h
 * \brief ASCII Serial Com Device
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
  ascii_serial_com asc; /**< used to receive messages and reply */
  char ascVersion; /**< This and next 4 variables just hold results, putting
                      them here lets them be statically allocated */
  char appVersion;
  char command;
  char dataBuffer[MAXDATALEN]; /**< data part of message received here */
  size_t dataLen;
  void (*frw)(ascii_serial_com *, char, char, char, char *data, size_t,
              void *); /**< called for r or w messages */
  void (*fs)(ascii_serial_com *, char, char, char, char *data, size_t,
             void *); /**< called for s messages */
  void (*fother)(ascii_serial_com *, char, char, char, char *data, size_t,
                 void *); /**< called for other messages */
  void *state_frw;
  void *state_fs;
  void *state_fother;
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
    ascii_serial_com_device *ascd,
    void (*frw)(ascii_serial_com *, char, char, char, char *data, size_t,
                void *), /**< called for r or w messages */
    void (*fs)(ascii_serial_com *, char, char, char, char *data, size_t,
               void *), /**< called for s messages */
    void (*fother)(ascii_serial_com *, char, char, char, char *data, size_t,
                   void *), /**< called for other messages */
    void *state_frw, void *state_fs, void *state_fother);

/** \brief ASCII Serial Com Device receive messages
 *
 * Receives ASCII Serial Com messages, and hands them over to the
 * appropriate one of the functions
 *
 * /param ascd: initialized ascii_serial_com_device
 *
 */
void ascii_serial_com_device_receive(ascii_serial_com_device *ascd);

/** \brief ASCII Serial Com Device get input buffer
 *
 * MAKE SURE ascd IS ALREADY INITIALIZED!
 *
 * /param ascd: initialized ascii_serial_com_device
 */
circular_buffer_uint8 *
ascii_serial_com_device_get_input_buffer(ascii_serial_com_device *ascd);

/** \brief ASCII Serial Com Device get output buffer
 *
 * MAKE SURE ascd IS ALREADY INITIALIZED!
 *
 * /param ascd: initialized ascii_serial_com_device
 */
circular_buffer_uint8 *
ascii_serial_com_device_get_output_buffer(ascii_serial_com_device *ascd);

#endif
