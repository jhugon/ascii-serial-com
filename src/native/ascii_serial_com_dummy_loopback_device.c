#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ascii_serial_com.h"
#include "circular_buffer.h"

#define bufCap 64
#define littleBufCap 8
circular_buffer_uint8 buffer;
uint8_t buffer_raw[bufCap];
uint8_t little_buffer[littleBufCap];

char dataBuffer[MAXDATALEN];

ascii_serial_com asc;

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

  struct pollfd fds[2] = {
      {infileno, POLLIN /*| POLLHUP*/, 0},
      {outfileno, POLLHUP, 0},
  };
  short *inflags = &fds[0].revents;
  short *outflags = &fds[1].revents;
  short *outsetflags = &fds[1].events;

  circular_buffer_init_uint8(&buffer, bufCap, buffer_raw);

  ascii_serial_com_init(&asc);
  circular_buffer_uint8 *asc_in_buf = ascii_serial_com_get_input_buffer(&asc);
  circular_buffer_uint8 *asc_out_buf = ascii_serial_com_get_output_buffer(&asc);

  while (true) {
    int ready = poll(fds, 2, -1);
    if (ready < 0) {
      perror("Error while polling");
      fprintf(stderr, "Exiting.\n");
      return 1;
    }
    if ((rawLoopback && circular_buffer_is_empty_uint8(&buffer)) ||
        (!rawLoopback && circular_buffer_is_empty_uint8(asc_out_buf) &&
         circular_buffer_is_empty_uint8(asc_in_buf))) {
      if (*inflags & POLLERR) {
        fprintf(stderr, "Infile error, exiting.\n");
        return 1;
      }
      if (*inflags & POLLHUP) {
        fprintf(stderr, "Infile hung up, exiting.\n");
        return 1;
      }
      if (*inflags & POLLNVAL) {
        fprintf(stderr, "Infile closed, exiting.\n");
        return 1;
      }
    }
    if (*outflags & POLLERR) {
      fprintf(stderr, "Outfile error, exiting.\n");
      return 1;
    }
    if (*outflags & POLLHUP) {
      fprintf(stderr, "Outfile hung up, exiting.\n");
      return 1;
    }
    if (*outflags & POLLNVAL) {
      fprintf(stderr, "Outfile closed, exiting.\n");
      return 1;
    }
    if (*inflags & POLLIN) {
      // read
      fprintf(stderr, "Something to read!\n");
      if (rawLoopback) {
        if (circular_buffer_is_full_uint8(&buffer)) {
          if (!usleep(1000)) {
            perror("Error while sleeping waiting for read");
            fprintf(stderr, "Exiting.\n");
            return 1;
          }
        } else {
          // fprintf(stderr,"About to call read()\n");
          ssize_t nRead = read(infileno, little_buffer, littleBufCap);
          // fprintf(stderr,"Read %zd\n",nRead);
          if (nRead < 0) {
            perror("Error reading from infile");
            fprintf(stderr, "Exiting.\n");
            return 1;
          }
          if (nRead == 0) {
            break;
          }
          for (ssize_t iChar = 0; iChar < nRead; iChar++) {
            circular_buffer_push_back_uint8(&buffer, little_buffer[iChar]);
          }
        }
      } else { // if rawLoopback
        circular_buffer_push_back_from_fd_uint8(asc_in_buf, infileno);
      } // else with if raw Loopback
    }   // if inflags POLLIN
    if (!rawLoopback && !circular_buffer_is_empty_uint8(asc_in_buf)) {
      fprintf(stderr, "About to try to receive message:\n");
      circular_buffer_print_uint8(asc_in_buf, stderr);
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
    // fprintf(stderr,"inflags: %hu outflags: %hu, buffer size: %zu\n",
    // *inflags, *outflags, circular_buffer_get_size_uint8(&buffer));
    if (*outflags & POLLOUT) {
      // write
      fprintf(stderr, "File ready to write!\n");
      if (rawLoopback) {
        if (circular_buffer_is_empty_uint8(&buffer)) {
          if (usleep(1000)) {
            perror("Error while sleeping waiting for write");
            fprintf(stderr, "Exiting.\n");
            return 1;
          }
        } else {
          while (!circular_buffer_is_empty_uint8(&buffer)) {
            const uint8_t c = circular_buffer_pop_front_uint8(&buffer);
            if (fputc(c, outfile) == EOF) {
              break;
            }
            if (fflush(outfile) == EOF) {
              perror("Error flushing after write");
              fprintf(stderr, "Exiting.\n");
              return 1;
            }
          }
        }
      } else { // if rawLoopback
               // circular_buffer_print_uint8(asc_out_buf, stderr);
        size_t nBytes =
            circular_buffer_pop_front_to_fd_uint8(asc_out_buf, outfileno);
        fprintf(stderr, "circular_buffer_pop_front_to_fd_uint8: %zu bytes\n",
                nBytes);
        // circular_buffer_print_uint8(asc_out_buf, stderr);
      }
    } // if outflags & POLLHUP
    // Should output POLLHUP?
    if (rawLoopback) {
      if (circular_buffer_is_empty_uint8(&buffer)) {
        *outsetflags = POLLHUP;
      } else {
        *outsetflags = POLLOUT | POLLHUP;
      }
    } else {
      if (circular_buffer_is_empty_uint8(asc_out_buf)) {
        *outsetflags = POLLHUP;
      } else {
        *outsetflags = POLLOUT | POLLHUP;
      }
    }
  }

  return 0;
}
