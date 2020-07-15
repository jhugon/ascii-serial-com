/** \brief ASCII Serial Com Device
 *
 */

#include "ascii_serial_com.h"

/** \brief ASCII Serial Com Interface State struct
 *
 *  Keeps track of the state of the ASCII Serial Com interface
 *
 *  The functions take the stuff from ascii_serial_com_receive plus a void
 * pointer to possible state/configuration info If the functions are null ptrs,
 * a check will be done and error message returned to host
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
