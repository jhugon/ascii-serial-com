#include "ascii_serial_com_device.h"
#include "stdio.h"

void ascii_serial_com_device_init(
    ascii_serial_com_device *ascd,
    void (*frw)(ascii_serial_com *, char, char, char, const char *data, size_t,
                void *), /**< called for r or w messages */
    void (*fs)(ascii_serial_com *, char, char, char, const char *data, size_t,
               void *), /**< called for s messages */
    void (*fother)(ascii_serial_com *, char, char, char, const char *data,
                   size_t, void *), /**< called for other messages */
    void *state_frw, void *state_fs, void *state_fother) {
  ascii_serial_com_init(&ascd->asc);
  ascd->frw = frw;
  ascd->fs = fs;
  ascd->fother = fother;
  ascd->state_frw = state_frw;
  ascd->state_fs = state_fs;
  ascd->state_fother = state_fother;
}

void ascii_serial_com_device_receive(ascii_serial_com_device *ascd) {

  char ascVersion, appVersion, command;
  size_t dataLen;
  ascii_serial_com_get_message_from_input_buffer(&ascd->asc, &ascVersion,
                                                 &appVersion, &command,
                                                 ascd->dataBuffer, &dataLen);
  fprintf(stderr, "device_receive: message type: %c", command);
  if (command == '\0') { // no message in input buffer
    // pass
  } else if (command == 'r' || command == 'w') {
    if (ascd->frw) {
      ascd->frw(&ascd->asc, ascVersion, appVersion, command, ascd->dataBuffer,
                dataLen, ascd->state_frw);
    }
  } else if (command == 's') {
    if (ascd->fs) {
      ascd->fs(&ascd->asc, ascVersion, appVersion, command, ascd->dataBuffer,
               dataLen, ascd->state_fs);
    }
  } else {
    if (ascd->fother) {
      ascd->fother(&ascd->asc, ascVersion, appVersion, command,
                   ascd->dataBuffer, dataLen, ascd->state_fother);
    }
  }
}
