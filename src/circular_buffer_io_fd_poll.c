#include "circular_buffer_io_fd_poll.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#define inflags(cb_io) cb_io->fds[0].revents
#define outflags(cb_io) cb_io->fds[1].revents
#define outsetflags(cb_io) cb_io->fds[1].events

void circular_buffer_io_fd_poll_init(circular_buffer_io_fd_poll *cb_io,
                                     circular_buffer_uint8 *in_buf,
                                     circular_buffer_uint8 *out_buf, int fd_in,
                                     int fd_out) {
  cb_io->in_buf = in_buf;
  cb_io->out_buf = out_buf;
  cb_io->fd_in = fd_in;
  cb_io->fd_out = fd_out;

  cb_io->fds[0] = (struct pollfd){fd_in, POLLIN /*| POLLHUP*/, 0};
  cb_io->fds[1] = (struct pollfd){fd_out, POLLHUP, 0};
}

uint8_t circular_buffer_io_fd_poll_do_poll(circular_buffer_io_fd_poll *cb_io,
                                           int timeout) {
  if (!circular_buffer_is_empty_uint8(cb_io->out_buf)) {
    outsetflags(cb_io) = POLLOUT | POLLHUP;
  } else {
    outsetflags(cb_io) = POLLHUP;
  }
  int ready = poll(cb_io->fds, 2, timeout);
  if (ready < 0) {
    perror("Error while polling");
    fprintf(stderr, "Exiting.\n");
    return 1;
  }
  if (circular_buffer_is_empty_uint8(cb_io->out_buf) &&
      circular_buffer_is_empty_uint8(cb_io->in_buf)) {
    if (inflags(cb_io) & POLLERR) {
      fprintf(stderr, "Infile error, exiting.\n");
      return 1;
    }
    if (inflags(cb_io) & POLLHUP) {
      fprintf(stderr, "Infile hung up, exiting.\n");
      return 1;
    }
    if (inflags(cb_io) & POLLNVAL) {
      fprintf(stderr, "Infile closed, exiting.\n");
      return 1;
    }
  }
  if (outflags(cb_io) & POLLERR) {
    fprintf(stderr, "Outfile error, exiting.\n");
    return 1;
  }
  if (outflags(cb_io) & POLLHUP) {
    fprintf(stderr, "Outfile hung up, exiting.\n");
    return 1;
  }
  if (outflags(cb_io) & POLLNVAL) {
    fprintf(stderr, "Outfile closed, exiting.\n");
    return 1;
  }

  return 0;
}

size_t circular_buffer_io_fd_poll_do_output(circular_buffer_io_fd_poll *cb_io) {
  if (!(outflags(cb_io) & POLLOUT)) { // out fd not ready for writing
    return 0;
  }
  return circular_buffer_pop_front_to_fd_uint8(cb_io->out_buf, cb_io->fd_out);
}

size_t circular_buffer_io_fd_poll_do_input(circular_buffer_io_fd_poll *cb_io) {
  if (!(inflags(cb_io) & POLLIN)) { // in fd not ready for reading
    return 0;
  }
  return circular_buffer_push_back_from_fd_uint8(cb_io->in_buf, cb_io->fd_in);
}

void circular_buffer_io_fd_poll_print(circular_buffer_io_fd_poll *cb_io,
                                      FILE *stream) {
  fprintf(stream,
          "circular_buffer_io_fd_poll: in buf: %p, out buf: %p, in fd: %i, out "
          "fd: %i\n",
          cb_io->in_buf, cb_io->out_buf, cb_io->fd_in, cb_io->fd_out);
  fprintf(stream, "%12s%12s%12s\n", "", "events", "revents"); // 12 each
  fprintf(stream, "%12s%12hi%12hi\n", "in POLLIN",
          (short)(cb_io->fds[0].events & POLLIN),
          (short)(cb_io->fds[0].revents & POLLIN));
  fprintf(stream, "%12s%12hi%12hi\n", "in POLLPRI",
          (short)(cb_io->fds[0].events & POLLPRI),
          (short)(cb_io->fds[0].revents & POLLPRI));
  fprintf(stream, "%12s%12hi%12hi\n", "in POLLOUT",
          (short)(cb_io->fds[0].events & POLLOUT),
          (short)(cb_io->fds[0].revents & POLLOUT));
  fprintf(stream, "%12s%12hi%12hi\n", "in POLLHUP",
          (short)(cb_io->fds[0].events & POLLHUP),
          (short)(cb_io->fds[0].revents & POLLHUP));
  fprintf(stream, "%12s%12hi%12hi\n", "out POLLIN",
          (short)(cb_io->fds[1].events & POLLIN),
          (short)(cb_io->fds[1].revents & POLLIN));
  fprintf(stream, "%12s%12hi%12hi\n", "out POLLPRI",
          (short)(cb_io->fds[1].events & POLLPRI),
          (short)(cb_io->fds[1].revents & POLLPRI));
  fprintf(stream, "%12s%12hi%12hi\n", "out POLLOUT",
          (short)(cb_io->fds[1].events & POLLOUT),
          (short)(cb_io->fds[1].revents & POLLOUT));
  fprintf(stream, "%12s%12hi%12hi\n", "out POLLHUP",
          (short)(cb_io->fds[1].events & POLLHUP),
          (short)(cb_io->fds[1].revents & POLLHUP));
  fprintf(stream, "%12s%12hi%12hi\n", "in raw", cb_io->fds[0].events,
          cb_io->fds[0].revents);
  fprintf(stream, "%12s%12hi%12hi\n", "out raw", cb_io->fds[1].events,
          cb_io->fds[1].revents);
}
