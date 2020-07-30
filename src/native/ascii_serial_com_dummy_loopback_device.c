#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ascii_serial_com.h"
#include "circular_buffer.h"
#include "circular_buffer_io_fd_poll.h"

#define bufCap 64
#define littleBufCap 8
circular_buffer_uint8 buffer;
uint8_t buffer_raw[bufCap];
uint8_t little_buffer[littleBufCap];

char dataBuffer[MAXDATALEN];

ascii_serial_com asc;

circular_buffer_io_fd_poll cb_io;

int main(int argc, char *argv[]) {

  bool rawLoopback = false;
  if (argc > 1) {
    if (strncmp("-h", argv[1], 2) == 0) {
      fprintf(stderr,
              "\n  ascii_serial_com_dummy_loopback_device [-h] [-l] <infile> "
              "<outfile>\n\n");
      fprintf(stderr,
              "  If no filenames are provided, then stdin and stdout are used\n"
              "  -h: show help and exit\n"
              "  -l: Raw loopback mode, ASCII Serial Com will not be used\n"
              "\n");
      return 0;
    }
    if (strncmp("-l", argv[1], 2) == 0) {
      fprintf(stderr,
              "\nRaw loopback mode enabled, ASCII Serial Com will not be "
              "used.\n\n");
      rawLoopback = true;
    }
  }
  if (argc == 2 && !rawLoopback) {
    fprintf(stderr, "Error: either 0 or 2 arguments required:\n");
    fprintf(stderr,
            "\n  ascii_serial_com_dummy_loopback_device [-h] [-l] <infile> "
            "<outfile>\n\n");
    fprintf(stderr,
            "  If no filenames are provided, then stdin and stdout are used\n"
            "  -h: show help and exit\n"
            "  -l: Raw loopback mode, ASCII Serial Com will not be used\n"
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

  circular_buffer_init_uint8(&buffer, bufCap, buffer_raw);

  ascii_serial_com_init(&asc);
  circular_buffer_uint8 *asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

  if (rawLoopback) {
    circular_buffer_io_fd_poll_init(&cb_io, &buffer, &buffer, infileno,
                                    outfileno);
  } else {
    circular_buffer_io_fd_poll_init(&cb_io, asc_in_buf, asc_out_buf, infileno,
                                    outfileno);
  }

  int timeout = -1;
  while (true) {
    int poll_ret_code = circular_buffer_io_fd_poll_do_poll(&cb_io, timeout);
    if (poll_ret_code != 0) {
      return 1;
    }
    circular_buffer_io_fd_poll_do_input(&cb_io);

    if (!rawLoopback && !circular_buffer_is_empty_uint8(asc_in_buf)) {
      // fprintf(stderr, "About to try to receive message:\n");
      // circular_buffer_print_uint8(asc_in_buf, stderr);
      char ascVersion, appVersion, command;
      size_t dataLen;
      ascii_serial_com_get_message_from_input_buffer(
          &asc, &ascVersion, &appVersion, &command, dataBuffer, &dataLen);
      if (command != '\0') {
        fprintf(stderr,
                "Received message:\n  asc and app versions: %c %c\n  command: "
                "%c dataLen: %zu\n  data: ",
                ascVersion, appVersion, command, dataLen);
        for (size_t iData = 0; iData < dataLen; iData++) {
          fprintf(stderr, "%c", dataBuffer[iData]);
        }
        fprintf(stderr, "\n");
        fflush(stderr);
        // circular_buffer_print_uint8(asc_out_buf, stderr);
        ascii_serial_com_put_message_in_output_buffer(
            &asc, ascVersion, appVersion, command, dataBuffer, dataLen);
      }
    }

    circular_buffer_io_fd_poll_do_input(&cb_io);

    // Do we need to process data in the input buffer?
    // If so, poll with short timeout, otherwise just poll
    // (all else is just waiting on IO)
    if (!rawLoopback && !circular_buffer_is_empty_uint8(asc_in_buf)) {
      timeout = 5; // ms
    } else {
      timeout = -1; // unlimited
    }
  }

  return 0;
}
