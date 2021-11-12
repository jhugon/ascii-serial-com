#include "ascii_serial_com_device.h"

/** \file */

void ascii_serial_com_device_init(
    ascii_serial_com_device *ascd,
    void (*frw)(ascii_serial_com *, char, char, char, char *data, size_t,
                void *), /**< called for r or w messages */
    void (*fs)(ascii_serial_com *, char, char, char, char *data, size_t,
               void *), /**< called for s messages */
    void (*fnf)(ascii_serial_com *, char, char, char, char *data, size_t,
                void *), /**< called for n and f messages */
    void (*fother)(ascii_serial_com *, char, char, char, char *data, size_t,
                   void *), /**< called for other messages */
    void *state_frw, void *state_fs, void *state_fnf, void *state_fother) {
  ascii_serial_com_init(&ascd->asc);
  ascd->frw = frw;
  ascd->fs = fs;
  ascd->fnf = fnf;
  ascd->fother = fother;
  ascd->state_frw = state_frw;
  ascd->state_fs = state_fs;
  ascd->state_fnf = state_fnf;
  ascd->state_fother = state_fother;
}

void ascii_serial_com_device_receive(ascii_serial_com_device *ascd) {

  ascii_serial_com_get_message_from_input_buffer(
      &ascd->asc, &ascd->ascVersion, &ascd->appVersion, &ascd->command,
      ascd->dataBuffer, &ascd->dataLen);
  if (ascd->command == '\0') { // no message in input buffer
    // pass
  } else if (ascd->command == 'r' || ascd->command == 'w') {
    if (ascd->frw) {
      ascd->frw(&ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
                ascd->dataBuffer, ascd->dataLen, ascd->state_frw);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  } else if (ascd->command == 's') {
    if (ascd->fs) {
      ascd->fs(&ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
               ascd->dataBuffer, ascd->dataLen, ascd->state_fs);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  } else if (ascd->command == 'n' || ascd->command == 'f') {
    if (ascd->fnf) {
      ascd->fnf(&ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
                ascd->dataBuffer, ascd->dataLen, ascd->state_fnf);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  } else {
    if (ascd->fother) {
      ascd->fother(&ascd->asc, ascd->ascVersion, ascd->appVersion,
                   ascd->command, ascd->dataBuffer, ascd->dataLen,
                   ascd->state_fother);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  }
}

circular_buffer_uint8 *
ascii_serial_com_device_get_input_buffer(ascii_serial_com_device *ascd) {
  return ascii_serial_com_get_input_buffer(&ascd->asc);
}

circular_buffer_uint8 *
ascii_serial_com_device_get_output_buffer(ascii_serial_com_device *ascd) {
  return ascii_serial_com_get_output_buffer(&ascd->asc);
}

void ascii_serial_com_device_put_message_in_output_buffer(
    ascii_serial_com_device *ascd, char ascVersion, char appVersion,
    char command, char *data, size_t dataLen) {
  return ascii_serial_com_put_message_in_output_buffer(
      &ascd->asc, ascVersion, appVersion, command, data, dataLen);
}
