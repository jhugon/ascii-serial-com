#include <stdio.h>

#include "ascii_serial_com.h"

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("Error: 2 arguments required:\n\n");
    printf("ascii_serial_com_dummy_loopback_device <infile> <outfile>\n\n");
    return 1;
  }
  const char *infilename = argv[1];
  const char *outfilename = argv[2];
  printf("infile: %s\noutfile: %s\n", infilename, outfilename);
  return 0;
}
