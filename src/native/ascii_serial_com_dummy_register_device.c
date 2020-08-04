#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "asc_exception.h"
#include "ascii_serial_com.h"
#include "ascii_serial_com_device.h"
#include "ascii_serial_com_register_block.h"
#include "circular_buffer_io_fd_poll.h"

char dataBuffer[MAXDATALEN];
#define nRegs 10
REGTYPE regs[nRegs];

circular_buffer_io_fd_poll cb_io;
ascii_serial_com_device ascd;
ascii_serial_com_register_block reg_block;

CEXCEPTION_T e;

int main(int argc, char *argv[]) {

  if (argc > 1) {
    if (strncmp("-h", argv[1], 2) == 0) {
      fprintf(stderr,
              "\n  ascii_serial_com_dummy_register_device [-h] <infile> "
              "<outfile>\n\n");
      fprintf(stderr,
              "  If no filenames are provided, then stdin and stdout are used\n"
              "  -h: show help and exit\n"
              "\n");
      return 0;
    }
  }
  if (argc != 3 && argc != 1) {
    fprintf(stderr, "Error: either 0 or 2 arguments required:\n");
    fprintf(stderr, "\n  ascii_serial_com_dummy_register_device [-h] <infile> "
                    "<outfile>\n\n");
    fprintf(stderr,
            "  If no filenames are provided, then stdin and stdout are used\n"
            "  -h: show help and exit\n"
            "\n");
    return 1;
  }

  FILE *infile;
  FILE *outfile;
  int infileno;
  int outfileno;
  if (argc == 3) {
    const char *infilename = argv[1];
    const char *outfilename = argv[2];
    fprintf(stdout, "infile: %s\noutfile: %s\n", infilename, outfilename);

    infile = fopen(infilename, "r");
    if (!infile) {
      perror("Error opening input file");
      fprintf(stderr, "Exiting.\n");
      return 1;
    }
    infileno = fileno(infile);
    if (infileno < 0) {
      perror("Error getting infile descriptor");
      fprintf(stderr, "Exiting.\n");
      return 1;
    }
    outfile = fopen(outfilename, "a+");
    if (!outfile) {
      perror("Error opening output file");
      fprintf(stderr, "Exiting.\n");
      return 1;
    }
    outfileno = fileno(outfile);
    if (outfileno < 0) {
      perror("Error getting infile descriptor");
      fprintf(stderr, "Exiting.\n");
      return 1;
    }

  } else { // no args
    fprintf(stderr, "infile: stdin\noutfile: stdout\n");
    infile = stdin;
    outfile = stdout;
    infileno = STDIN_FILENO;
    outfileno = STDOUT_FILENO;
  }

  ascii_serial_com_register_block_init(&reg_block, regs, nRegs);
  ascii_serial_com_device_init(&ascd,
                               ascii_serial_com_register_block_handle_message,
                               NULL, NULL, &reg_block, NULL, NULL);
  circular_buffer_uint8 *asc_in_buf =
      ascii_serial_com_device_get_input_buffer(&ascd);
  circular_buffer_uint8 *asc_out_buf =
      ascii_serial_com_device_get_output_buffer(&ascd);

  circular_buffer_io_fd_poll_init(&cb_io, asc_in_buf, asc_out_buf, infileno,
                                  outfileno);

  int timeout = -1;
  while (true) {
    Try {
      int poll_ret_code = circular_buffer_io_fd_poll_do_poll(&cb_io, timeout);
      if (poll_ret_code != 0) {
        return 1;
      }
      circular_buffer_io_fd_poll_do_input(&cb_io);

      ascii_serial_com_device_receive(&ascd);

      circular_buffer_io_fd_poll_do_output(&cb_io);

      // Do we need to process data in the input buffer?
      // If so, poll with short timeout, otherwise just poll
      // (all else is just waiting on IO)
      if (!circular_buffer_is_empty_uint8(asc_in_buf)) {
        timeout = 5; // ms
      } else {
        timeout = -1; // unlimited
      }
    }
    Catch(e) {
      fprintf(stderr, "Uncaught exception: %u, exiting.\n", e);
      return 1;
    }
  }

  return 0;
}
