#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include "ascii_serial_com.h"
#include "circular_buffer.h"

#define bufCap 64
circular_buffer_uint8 buffer;
uint8_t buffer_raw[bufCap];

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("Error: 2 arguments required:\n\n");
    printf("ascii_serial_com_dummy_loopback_device <infile> <outfile>\n\n");
    return 1;
  }
  const char *infilename = argv[1];
  const char *outfilename = argv[2];
  printf("infile: %s\noutfile: %s\n", infilename, outfilename);

  FILE *infile = fopen(infilename, "r");
  if (!infile) {
    perror("Error opening input file");
    printf("Exiting.\n");
    return 1;
  }
  int infileno = fileno(infile);
  if (infileno < 0) {
    perror("Error getting infile descriptor");
    printf("Exiting.\n");
    return 1;
  }

  FILE *outfile = fopen(outfilename, "a+");
  if (!outfile) {
    perror("Error opening output file");
    printf("Exiting.\n");
    return 1;
  }
  int outfileno = fileno(outfile);
  if (outfileno < 0) {
    perror("Error getting infile descriptor");
    printf("Exiting.\n");
    return 1;
  }

  struct pollfd fds[2] = {
      {infileno, POLLIN | POLLHUP, 0},
      {outfileno, POLLOUT | POLLHUP, 0},
  };
  short *inflags = &fds[0].revents;
  short *outflags = &fds[0].revents;

  circular_buffer_init_uint8(&buffer, bufCap, buffer_raw);

  while (true) {
    int ready = poll(fds, 2, -1);
    if (ready < 0) {
      perror("Error while polling");
      printf("Exiting.\n");
      return 1;
    }
    if ((*inflags & POLLERR) > 0) {
      printf("Infile error, exiting.\n");
    }
    if ((*outflags & POLLERR) > 0) {
      printf("Outfile error, exiting.\n");
    }
    if ((*inflags & POLLHUP) > 0) {
      printf("Infile hung up, exiting.\n");
    }
    if ((*outflags & POLLHUP) > 0) {
      printf("Outfile hung up, exiting.\n");
    }
    if ((*inflags & POLLNVAL) > 0) {
      printf("Infile closed, exiting.\n");
    }
    if ((*outflags & POLLNVAL) > 0) {
      printf("Outfile closed, exiting.\n");
    }
    if ((*inflags & POLLIN) > 0) {
      // read
      if (circular_buffer_is_full_uint8(&buffer)) {
        if (!usleep(1000)) {
          perror("Error while sleeping waiting for read");
          printf("Exiting.\n");
          return 1;
        }
      } else {
        const uint8_t c = fgetc(infile);
        circular_buffer_push_back_uint8(&buffer, c);
      }
    }
    if ((*outflags & POLLOUT) > 0) {
      // write
      if (circular_buffer_is_empty_uint8(&buffer)) {
        if (!usleep(1000)) {
          perror("Error while sleeping waiting for read");
          printf("Exiting.\n");
          return 1;
        }
      } else {
        const uint8_t c = circular_buffer_pop_front_uint8(&buffer);
        fputc(c, outfile);
      }
    }
  }

  return 0;
}
