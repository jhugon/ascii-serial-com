#include "circular_buffer_io_fd_poll.h"
#include "externals/unity.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define bufsize 8
uint8_t in_raw_buf[bufsize];
uint8_t out_raw_buf[bufsize];

circular_buffer_uint8 in_buf;
circular_buffer_uint8 out_buf;

circular_buffer_io_fd_poll cb_io;

void setUp(void) {
  // set stuff up here
  circular_buffer_init_uint8(&in_buf, bufsize, in_raw_buf);
  circular_buffer_init_uint8(&out_buf, bufsize, out_raw_buf);
}

void tearDown(void) {
  // clean stuff up here
}

void test_circular_buffer_io_fd_poll_init(void) {
  const int infd = open("/dev/zero", O_RDONLY);
  if (infd == -1) {
    perror("Couldn't open /dev/zero");
    TEST_FAIL_MESSAGE("Couldn't open input file /dev/zero");
  }
  const int outfd = open("/dev/null", O_WRONLY | O_APPEND);
  if (outfd == -1) {
    perror("Couldn't open /dev/null");
    TEST_FAIL_MESSAGE("Couldn't open output file /dev/null");
  }

  circular_buffer_io_fd_poll_init(&cb_io, &in_buf, &out_buf, infd, outfd);

  TEST_ASSERT_EQUAL_PTR(&in_buf, cb_io.in_buf);
  TEST_ASSERT_EQUAL_PTR(&out_buf, cb_io.out_buf);
  TEST_ASSERT_EQUAL(infd, cb_io.fd_in);
  TEST_ASSERT_EQUAL(outfd, cb_io.fd_out);
}

void test_circular_buffer_io_fd_poll_do_poll(void) {
  const int infd = open("/dev/zero", O_RDONLY);
  if (infd == -1) {
    perror("Couldn't open /dev/zero");
    TEST_FAIL_MESSAGE("Couldn't open input file /dev/zero");
  }
  const int outfd = open("/dev/null", O_WRONLY | O_APPEND);
  if (outfd == -1) {
    perror("Couldn't open /dev/null");
    TEST_FAIL_MESSAGE("Couldn't open output file /dev/null");
  }

  circular_buffer_io_fd_poll_init(&cb_io, &in_buf, &out_buf, infd, outfd);

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }

  // pollin always set, pollout not set b/c out buf empty
  // circular_buffer_io_fd_poll_print(&cb_io,stderr);
  TEST_ASSERT_EQUAL(1, (cb_io.fds[0].events & POLLIN) > 0); // input pollin
  TEST_ASSERT_EQUAL(1, (cb_io.fds[0].revents & POLLIN) > 0);
  TEST_ASSERT_EQUAL(0, (cb_io.fds[1].events & POLLOUT) > 0); // output pollout
  TEST_ASSERT_EQUAL(0, (cb_io.fds[1].revents & POLLOUT) > 0);

  circular_buffer_push_back_uint8(&out_buf, 'x');

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }

  // pollin always set, pollout set b/c out buf not empty
  // circular_buffer_io_fd_poll_print(&cb_io,stderr);
  TEST_ASSERT_EQUAL(1, (cb_io.fds[0].events & POLLIN) > 0); // input pollin
  TEST_ASSERT_EQUAL(1, (cb_io.fds[0].revents & POLLIN) > 0);
  TEST_ASSERT_EQUAL(1, (cb_io.fds[1].events & POLLOUT) > 0); // output pollout
  TEST_ASSERT_EQUAL(1, (cb_io.fds[1].revents & POLLOUT) > 0);

  circular_buffer_pop_back_uint8(&out_buf);

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }

  // pollin always set, pollout not set b/c out buf empty
  // circular_buffer_io_fd_poll_print(&cb_io,stderr);
  TEST_ASSERT_EQUAL(1, (cb_io.fds[0].events & POLLIN) > 0); // input pollin
  TEST_ASSERT_EQUAL(1, (cb_io.fds[0].revents & POLLIN) > 0);
  TEST_ASSERT_EQUAL(0, (cb_io.fds[1].events & POLLOUT) > 0); // output pollout
  TEST_ASSERT_EQUAL(0, (cb_io.fds[1].revents & POLLOUT) > 0);
}

void test_circular_buffer_io_fd_poll_do_input(void) {
  const int infd = open("/dev/zero", O_RDONLY);
  if (infd == -1) {
    perror("Couldn't open /dev/zero");
    TEST_FAIL_MESSAGE("Couldn't open input file /dev/zero");
  }
  const int outfd = open("/dev/null", O_WRONLY | O_APPEND);
  if (outfd == -1) {
    perror("Couldn't open /dev/null");
    TEST_FAIL_MESSAGE("Couldn't open output file /dev/null");
  }

  circular_buffer_io_fd_poll_init(&cb_io, &in_buf, &out_buf, infd, outfd);

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }

  size_t nRead = circular_buffer_io_fd_poll_do_input(&cb_io);
  TEST_ASSERT_EQUAL_size_t(bufsize, nRead);
  for (size_t i = 0; i < bufsize; i++) {
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_pop_front_uint8(&in_buf));
  }

  for (size_t i = 0; i < 5; i++) {
    circular_buffer_push_back_uint8(&in_buf, 1);
  }

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }

  nRead = circular_buffer_io_fd_poll_do_input(&cb_io);
  TEST_ASSERT_GREATER_THAN_size_t(0, nRead);
}

void test_circular_buffer_io_fd_poll_do_output(void) {
  const int infd = open("/dev/zero", O_RDONLY);
  if (infd == -1) {
    perror("Couldn't open /dev/zero");
    TEST_FAIL_MESSAGE("Couldn't open input file /dev/zero");
  }
  const int outfd = open("/dev/null", O_WRONLY | O_APPEND);
  if (outfd == -1) {
    perror("Couldn't open /dev/null");
    TEST_FAIL_MESSAGE("Couldn't open output file /dev/null");
  }

  circular_buffer_io_fd_poll_init(&cb_io, &in_buf, &out_buf, infd, outfd);

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }
  size_t nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
  TEST_ASSERT_EQUAL_size_t(0, nWritten);

  for (size_t i = 0; i < bufsize; i++) {
    circular_buffer_push_back_uint8(&out_buf, i);
  }
  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }
  nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
  TEST_ASSERT_EQUAL_size_t(bufsize, nWritten);
  TEST_ASSERT_EQUAL_size_t(0, circular_buffer_get_size_uint8(&out_buf));

  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }
  nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
  TEST_ASSERT_EQUAL_size_t(0, nWritten);

  for (size_t i = 0; i < bufsize / 2; i++) {
    circular_buffer_push_back_uint8(&out_buf, i);
  }
  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }
  nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
  TEST_ASSERT_EQUAL_size_t(bufsize / 2, nWritten);
  TEST_ASSERT_EQUAL_size_t(0, circular_buffer_get_size_uint8(&out_buf));

  for (size_t i = 0; i < 4 * bufsize + bufsize / 2; i++) {
    circular_buffer_push_back_uint8(&out_buf, i);
  }
  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }
  nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
  TEST_ASSERT_EQUAL_size_t(bufsize / 2, nWritten);
  TEST_ASSERT_EQUAL_size_t(bufsize / 2,
                           circular_buffer_get_size_uint8(&out_buf));
  if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
    TEST_FAIL_MESSAGE("Error while polling");
  }
  nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
  TEST_ASSERT_EQUAL_size_t(bufsize / 2, nWritten);
  TEST_ASSERT_EQUAL_size_t(0, circular_buffer_get_size_uint8(&out_buf));
}

void test_circular_buffer_io_fd_poll_do_in_and_out(void) {
  const int infd = open("/dev/zero", O_RDONLY);
  if (infd == -1) {
    perror("Couldn't open /dev/zero");
    TEST_FAIL_MESSAGE("Couldn't open input file /dev/zero");
  }
  const int outfd = open("/dev/null", O_WRONLY | O_APPEND);
  if (outfd == -1) {
    perror("Couldn't open /dev/null");
    TEST_FAIL_MESSAGE("Couldn't open output file /dev/null");
  }

  circular_buffer_io_fd_poll_init(&cb_io, &in_buf, &out_buf, infd, outfd);

  for (size_t i = 0; i < 10; i++) {
    if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
      TEST_FAIL_MESSAGE("Error while polling");
    }
    size_t nRead = circular_buffer_io_fd_poll_do_input(&cb_io);
    TEST_ASSERT_EQUAL_size_t(bufsize, nRead);
    size_t inbufsize = circular_buffer_get_size_uint8(&in_buf);
    TEST_ASSERT_EQUAL_size_t(bufsize, inbufsize);
    for (size_t j = 0; j < inbufsize; j++) {
      circular_buffer_push_back_uint8(&out_buf,
                                      circular_buffer_pop_front_uint8(&in_buf));
    }
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_get_size_uint8(&in_buf));
    TEST_ASSERT_EQUAL_size_t(inbufsize,
                             circular_buffer_get_size_uint8(&out_buf));
    /////
    if (circular_buffer_io_fd_poll_do_poll(&cb_io, 20) != 0) {
      TEST_FAIL_MESSAGE("Error while polling");
    }
    size_t nWritten = circular_buffer_io_fd_poll_do_output(&cb_io);
    TEST_ASSERT_EQUAL_size_t(bufsize, nWritten);
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_get_size_uint8(&in_buf));
    TEST_ASSERT_EQUAL_size_t(0, circular_buffer_get_size_uint8(&out_buf));
  }
}

void test_circular_buffer_io_fd_poll_print(void) {
  const int infd = open("/dev/zero", O_RDONLY);
  if (infd == -1) {
    perror("Couldn't open /dev/zero");
    TEST_FAIL_MESSAGE("Couldn't open input file /dev/zero");
  }
  const int outfd = open("/dev/null", O_WRONLY | O_APPEND);

  circular_buffer_io_fd_poll_init(&cb_io, &in_buf, &out_buf, outfd, outfd);

  FILE *devnull = fopen("/dev/null", "w");
  if (devnull == NULL) {
    perror("Error opening /dev/null");
    TEST_FAIL_MESSAGE("Error opening /dev/null");
  }

  circular_buffer_io_fd_poll_print(&cb_io, devnull);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_circular_buffer_io_fd_poll_init);
  RUN_TEST(test_circular_buffer_io_fd_poll_do_poll);
  RUN_TEST(test_circular_buffer_io_fd_poll_do_input);
  RUN_TEST(test_circular_buffer_io_fd_poll_do_output);
  RUN_TEST(test_circular_buffer_io_fd_poll_do_in_and_out);
  RUN_TEST(test_circular_buffer_io_fd_poll_print);
  return UNITY_END();
}
