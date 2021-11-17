#include "ascii_serial_com_device.h"

/** \file */

void ascii_serial_com_device_init(ascii_serial_com_device *ascd,
                                  ascii_serial_com_device_config *config) {
  ascii_serial_com_init(&ascd->asc);
  ascd->config = config;
}

void ascii_serial_com_device_receive(ascii_serial_com_device *ascd) {

  ascii_serial_com_get_message_from_input_buffer(
      &ascd->asc, &ascd->ascVersion, &ascd->appVersion, &ascd->command,
      ascd->dataBuffer, &ascd->dataLen);
  if (ascd->command == '\0') { // no message in input buffer
    // pass
  } else if (ascd->command == 'r' || ascd->command == 'w') {
    if (ascd->config->func_rw) {
      ascd->config->func_rw(&ascd->asc, ascd->ascVersion, ascd->appVersion,
                            ascd->command, ascd->dataBuffer, ascd->dataLen,
                            ascd->config->state_rw);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  } else if (ascd->command == 's') {
    if (ascd->config->func_s) {
      ascd->config->func_s(&ascd->asc, ascd->ascVersion, ascd->appVersion,
                           ascd->command, ascd->dataBuffer, ascd->dataLen,
                           ascd->config->state_s);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  } else if (ascd->command == 'n' || ascd->command == 'f') {
    if (ascd->config->func_nf) {
      ascd->config->func_nf(&ascd->asc, ascd->ascVersion, ascd->appVersion,
                            ascd->command, ascd->dataBuffer, ascd->dataLen,
                            ascd->config->state_nf);
    } else {
      ascii_serial_com_put_error_in_output_buffer(
          &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
          ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
    }
  } else {
    ascii_serial_com_put_error_in_output_buffer(
        &ascd->asc, ascd->ascVersion, ascd->appVersion, ascd->command,
        ascd->dataBuffer, ascd->dataLen, ASC_ERROR_COMMAND_NOT_IMPLEMENTED);
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
