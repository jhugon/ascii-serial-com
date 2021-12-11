#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "circular_buffer.h"

ascii_serial_com asc;
circular_buffer_uint8 *asc_in_buf;
circular_buffer_uint8 *asc_out_buf;

CEXCEPTION_T e;
char ascVersion, appVersion, command;
size_t dataLen;
char dataBuffer[MAXDATALEN];
unsigned long iIteration;

char usage[] = "\n  ascii_serial_com_encode_decode_profiler [-h] <N>\n\n"
               "  where N is the number of iterations to run\n"
               "  -h: show this help then exit\n"
               "\n\0";

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr,
            "ascii_serial_com_encode_decode_profiler not enough arguments\n");
    fprintf(stderr, "%s", usage);
    return 1;
  }
  if (strncmp("-h", argv[1], 2) == 0) {
    fprintf(stderr, "%s", usage);
    return 0;
  }
  const unsigned long nIterations = strtoul(argv[1], NULL, 10);
  fprintf(stdout, "Running with %lu iterations.\n", nIterations);

  Try {
    ascii_serial_com_init(&asc);
    asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
    asc_out_buf = ascii_serial_com_get_output_buffer(&asc);
  }
  Catch(e) {
    fprintf(stderr, "Uncaught exception: %u, during init, exiting.\n", e);
    return 1;
  }

  for (iIteration = 0; iIteration < nIterations; iIteration++) {
    Try {
      // fprintf(stdout,"Iteration %lu\n",iIteration);
      ascii_serial_com_put_message_in_output_buffer(&asc, '0', '0', 'z', NULL,
                                                    0);
      const size_t messageLen = circular_buffer_get_size_uint8(asc_out_buf);
      for (size_t iChar = 0; iChar < messageLen; iChar++) {
        circular_buffer_push_back_uint8(
            asc_in_buf, circular_buffer_pop_front_uint8(asc_out_buf));
      }
      ascii_serial_com_get_message_from_input_buffer(
          &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
    }
    Catch(e) { fprintf(stderr, "Uncaught exception: %u, continuing.\n", e); }
  }

  fprintf(stdout, "Finished %lu iterations.\n", nIterations);
  return 0;
}
