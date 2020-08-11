#ifndef CIRCULAR_BUFFER_IO_FD_POLL_H
#define CIRCULAR_BUFFER_IO_FD_POLL_H

/** \file
 * \brief Circular buffer IO with file descriptor polling
 *
 * Usage:
 *
 * Initialize circular buffers (or ASCII Serial Com object), and input and
 * output files.
 *
 * In the event loop:
 *
 * Run circular_buffer_io_fd_poll_do_poll
 *
 * Run circular_buffer_io_fd_poll_do_input
 *
 * Run circular_buffer_io_fd_poll_do_output
 *
 * Process data and/or push/pop from circular buffers either after running
 * do_input or after running do_output
 */

#include "circular_buffer.h"
#include <poll.h>
#include <stdio.h>

/** \brief Circular buffer IO with file descriptor polling struct
 *
 *  Keeps track of the state of the IO
 *
 */
typedef struct circular_buffer_io_fd_poll_struct {
  circular_buffer_uint8 *in_buf;
  circular_buffer_uint8 *out_buf;
  int fd_in;
  int fd_out;
  struct pollfd fds[2]; // fds[0] is in, fds[1] is out
} circular_buffer_io_fd_poll;

/** \brief Initialize circular buffer IO with file descriptor polling object
 *
 * Initialize object
 *
 * \param uninitialized circular_buffer_io_fd_poll object
 *
 * All other arguments should already be initialized
 *
 */
void circular_buffer_io_fd_poll_init(circular_buffer_io_fd_poll *cb_io,
                                     circular_buffer_uint8 *in_buf,
                                     circular_buffer_uint8 *out_buf, int fd_in,
                                     int fd_out);

/** \brief poll circular buffer IO with file descriptor polling object
 *
 * Poll file descriptors. Always polls on input file descriptor, but only polls
 * on output file descriptor when output circular buffer is not empty.
 *
 * \param initialized circular_buffer_io_fd_poll object
 *
 * \param timeout: timout after waiting for input or output fds for this long,
 * in ms. If 0, return immediately. If -1, wait indefinetly for fds to be
 * ready. See poll documentation (man 2 poll)
 *
 * \return status: if 0 success, otherwise failure
 */
uint8_t circular_buffer_io_fd_poll_do_poll(circular_buffer_io_fd_poll *cb_io,
                                           int timeout);

/** \brief circular buffer IO with file descriptor polling object: write to
 * output fd
 *
 * Write to output fd from output circular buffer. Only does anything if output
 * circular buffer not empty and output fd ready for write, the latter of which
 * is taken care of by circular_buffer_io_fd_poll_do_poll.
 *
 * Elements are popped from the front of the output circular buffer.
 *
 * \param initialized circular_buffer_io_fd_poll object
 *
 * \return the number of elements written to the fd and popped from output
 * circular buffer
 */
size_t circular_buffer_io_fd_poll_do_output(circular_buffer_io_fd_poll *cb_io);

/** \brief circular buffer IO with file descriptor polling object: read from
 * input fd
 *
 * Read from input fd to input circular buffer. Only does anything if input fd
 * has something to read, which is taken care of by
 * circular_buffer_io_fd_poll_do_poll.
 *
 * Elements are pushed to the back of the input circular buffer.
 *
 * \param initialized circular_buffer_io_fd_poll object
 *
 * \return the number of elements read from the fd and pushed to the input
 * circular buffer
 */
size_t circular_buffer_io_fd_poll_do_input(circular_buffer_io_fd_poll *cb_io);

void circular_buffer_io_fd_poll_print(circular_buffer_io_fd_poll *cb_io,
                                      FILE *stream);

#endif
