#include "circular_buffer.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// Private helper functions

inline void inc_iStart_uint8(circular_buffer_uint8 *buf);
inline void inc_iStop_uint8(circular_buffer_uint8 *buf);
inline void dec_iStart_uint8(circular_buffer_uint8 *buf);
inline void dec_iStop_uint8(circular_buffer_uint8 *buf);

void inc_iStart_uint8(circular_buffer_uint8 *buf) {
  buf->iStart = (buf->iStart + 1) % buf->capacity;
}

void inc_iStop_uint8(circular_buffer_uint8 *buf) {
  buf->iStop = (buf->iStop + 1) % buf->capacity;
}

void dec_iStart_uint8(circular_buffer_uint8 *buf) {
  if (buf->iStart == 0) {
    buf->iStart = buf->capacity - 1;
  } else {
    buf->iStart = (buf->iStart - 1) % buf->capacity;
  }
}

void dec_iStop_uint8(circular_buffer_uint8 *buf) {
  if (buf->iStop == 0) {
    buf->iStop = buf->capacity - 1;
  } else {
    buf->iStop = (buf->iStop - 1) % buf->capacity;
  }
}

void circular_buffer_init_uint8(circular_buffer_uint8 *circ_buf,
                                const size_t capacity, uint8_t *buffer) {
  circ_buf->capacity = capacity;
  circ_buf->size = 0;
  circ_buf->iStart = 0;
  circ_buf->iStop = 0;
  circ_buf->buffer = buffer;
}

size_t circular_buffer_get_size_uint8(const circular_buffer_uint8 *circ_buf) {
  return circ_buf->size;
}

bool circular_buffer_is_full_uint8(const circular_buffer_uint8 *circ_buf) {
  return circular_buffer_get_size_uint8(circ_buf) == circ_buf->capacity;
}

bool circular_buffer_is_empty_uint8(const circular_buffer_uint8 *circ_buf) {
  return circular_buffer_get_size_uint8(circ_buf) == 0;
}

void circular_buffer_print_uint8(const circular_buffer_uint8 *circ_buf,
                                 FILE *outfile) {

  fprintf(outfile, "circular_buffer_uint8, capacity: %zu\n",
          circ_buf->capacity);
  fprintf(outfile, "  size: %zu iStart: %zu iStop %zu buffer: %p\n",
          circ_buf->size, circ_buf->iStart, circ_buf->iStop,
          (void *)circ_buf->buffer);
  fprintf(outfile, "  Content: [ ");
  for (size_t i = 0; i < circ_buf->size; i++) {
    fprintf(outfile, "%" PRIu8 " ",
            circular_buffer_get_element_uint8(circ_buf, i));
  }
  fprintf(outfile, "]\n");
  fprintf(outfile, "  Raw Memory: [ ");
  for (size_t i = 0; i < circ_buf->capacity; i++) {
    fprintf(outfile, "%" PRIu8 " ", *(circ_buf->buffer + i));
  }
  fprintf(outfile, "]\n");
  fprintf(outfile, "  Content as string: ");
  for (size_t i = 0; i < circ_buf->size; i++) {
    uint8_t thisChar = circular_buffer_get_element_uint8(circ_buf, i);
    if (thisChar < 0x20 || thisChar >= 0x7F) { // is control char
      fprintf(outfile, "\\x%02" PRIX8, thisChar);
    } else { // is printable
      fprintf(outfile, "%c", thisChar);
    }
  }
  fprintf(outfile, "\n");
  fprintf(outfile, "  Raw memory as string: ");
  for (size_t i = 0; i < circ_buf->capacity; i++) {
    uint8_t thisChar = *(circ_buf->buffer + i);
    if (thisChar < 0x20 || thisChar >= 0x7F) { // is control char
      fprintf(outfile, "\\x%02" PRIX8, thisChar);
    } else { // is printable
      fprintf(outfile, "%c", thisChar);
    }
  }
  fprintf(outfile, "\n");
}

uint8_t circular_buffer_get_element_uint8(const circular_buffer_uint8 *circ_buf,
                                          const size_t iElement) {
  assert(iElement < circular_buffer_get_size_uint8(circ_buf));
  size_t iResult = (circ_buf->iStart + iElement) % circ_buf->capacity;
  uint8_t result = *(circ_buf->buffer + iResult);
  return result;
}

void circular_buffer_push_front_uint8(circular_buffer_uint8 *circ_buf,
                                      const uint8_t element) {
  dec_iStart_uint8(circ_buf);
  *(circ_buf->buffer + circ_buf->iStart) = element;
  if (circ_buf->size == circ_buf->capacity) {
    dec_iStop_uint8(circ_buf);
  } else {
    circ_buf->size++;
  }
}

void circular_buffer_push_back_uint8(circular_buffer_uint8 *circ_buf,
                                     const uint8_t element) {
  *(circ_buf->buffer + circ_buf->iStop) = element;
  inc_iStop_uint8(circ_buf);
  if (circ_buf->size == circ_buf->capacity) {
    inc_iStart_uint8(circ_buf);
  } else {
    circ_buf->size++;
  }
}

uint8_t circular_buffer_pop_front_uint8(circular_buffer_uint8 *circ_buf) {
  assert(circ_buf->size > 0);
  const uint8_t result = *(circ_buf->buffer + circ_buf->iStart);
  inc_iStart_uint8(circ_buf);
  circ_buf->size--;
  return result;
}

uint8_t circular_buffer_pop_back_uint8(circular_buffer_uint8 *circ_buf) {
  assert(circ_buf->size > 0);
  dec_iStop_uint8(circ_buf);
  circ_buf->size--;
  const uint8_t result = *(circ_buf->buffer + circ_buf->iStop);
  return result;
}

void circular_buffer_remove_front_to_uint8(circular_buffer_uint8 *circ_buf,
                                           const uint8_t value,
                                           const bool inclusive) {
  while (circ_buf->size > 0) {
    uint8_t iValue = circular_buffer_get_element_uint8(circ_buf, 0);
    if (iValue == value) {
      if (inclusive) {
        circular_buffer_pop_front_uint8(circ_buf);
      }
      return;
    } else {
      circular_buffer_pop_front_uint8(circ_buf);
    }
  }
}

void circular_buffer_remove_back_to_uint8(circular_buffer_uint8 *circ_buf,
                                          const uint8_t value,
                                          const bool inclusive) {
  while (circ_buf->size > 0) {
    uint8_t iValue =
        circular_buffer_get_element_uint8(circ_buf, circ_buf->size - 1);
    if (iValue == value) {
      if (inclusive) {
        circular_buffer_pop_back_uint8(circ_buf);
      }
      return;
    } else {
      circular_buffer_pop_back_uint8(circ_buf);
    }
  }
}

size_t circular_buffer_find_first_uint8(const circular_buffer_uint8 *circ_buf,
                                        const uint8_t value) {
  size_t iElement = 0;
  for (iElement = 0; iElement < circ_buf->size; iElement++) {
    uint8_t iValue = circular_buffer_get_element_uint8(circ_buf, iElement);
    if (iValue == value) {
      return iElement;
    }
  }
  return circ_buf->size;
}

size_t circular_buffer_find_last_uint8(const circular_buffer_uint8 *circ_buf,
                                       const uint8_t value) {
  size_t iElement = circ_buf->size;
  while (iElement > 0) {
    iElement--;
    uint8_t iValue = circular_buffer_get_element_uint8(circ_buf, iElement);
    if (iValue == value) {
      return iElement;
    }
  }
  return circ_buf->size;
}

size_t circular_buffer_count_uint8(const circular_buffer_uint8 *circ_buf,
                                   const uint8_t value) {
  size_t result = 0;
  for (size_t iElement = 0; iElement < circ_buf->size; iElement++) {
    uint8_t iValue = circular_buffer_get_element_uint8(circ_buf, iElement);
    if (iValue == value) {
      result++;
    }
  }
  return result;
}

size_t
circular_buffer_get_first_block_uint8(const circular_buffer_uint8 *circ_buf,
                                      const uint8_t **outBlock) {
  *outBlock = circ_buf->buffer + circ_buf->iStart;
  if (circ_buf->iStart + circ_buf->size > circ_buf->capacity) { // wraps
    return circ_buf->capacity - circ_buf->iStart;
  } else { // doesn't wrap
    return circ_buf->size;
  }
}

size_t
circular_buffer_delete_first_block_uint8(circular_buffer_uint8 *circ_buf) {
  if (circ_buf->iStart + circ_buf->size > circ_buf->capacity) { // wraps
    const size_t toPop = circ_buf->capacity - circ_buf->iStart;
    for (size_t i = 0; i < toPop; i++) {
      circular_buffer_pop_front_uint8(circ_buf);
    }
    return toPop;
  } else { // doesn't wrap
    const size_t origSize = circ_buf->size;
    circ_buf->size = 0;
    circ_buf->iStart = 0;
    circ_buf->iStop = 0;
    return origSize;
  }
}

void circular_buffer_push_back_block_uint8(circular_buffer_uint8 *circ_buf,
                                           const uint8_t *source,
                                           size_t source_size) {
  for (size_t iElement = 0; iElement < source_size; iElement++) {
    circular_buffer_push_back_uint8(circ_buf, source[iElement]);
  }
}

size_t circular_buffer_pop_front_block_uint8(circular_buffer_uint8 *circ_buf,
                                             uint8_t *destination,
                                             size_t dest_size) {
  size_t nPopped = circular_buffer_get_size_uint8(circ_buf);
  if (nPopped > dest_size) {
    nPopped = dest_size;
  }
  for (size_t iElement = 0; iElement < nPopped; iElement++) {
    destination[iElement] = circular_buffer_pop_front_uint8(circ_buf);
  }
  return nPopped;
}

size_t circular_buffer_push_back_from_fd_uint8(circular_buffer_uint8 *circ_buf,
                                               int fd) {
  size_t block_size_available;
  uint8_t *block_start = circ_buf->buffer + circ_buf->iStop;
  if (circ_buf->iStop >= circ_buf->iStart) {
    block_size_available = circ_buf->capacity - circ_buf->iStop;
  } else {
    block_size_available = circ_buf->iStart - circ_buf->iStop;
  }
  ssize_t nRead = read(fd, block_start, block_size_available);
  if (nRead < 0) {
    perror("circular_buffer_push_back_from_fd_uint8 error while reading from "
           "file");
    exit(1);
  }
  circ_buf->iStop = (circ_buf->iStop + nRead) % circ_buf->capacity;
  circ_buf->size += nRead;
  return nRead;
}

size_t circular_buffer_pop_front_to_fd_uint8(circular_buffer_uint8 *circ_buf,
                                             const int fd) {
  uint8_t *outBlock = circ_buf->buffer + circ_buf->iStart;
  size_t nBlock;
  if (circ_buf->iStart + circ_buf->size > circ_buf->capacity) { // wraps
    nBlock = circ_buf->capacity - circ_buf->iStart;
  } else { // doesn't wrap
    nBlock = circ_buf->size;
  }
  ssize_t nWritten = write(fd, outBlock, nBlock);
  if (nWritten < 0) {
    perror("circular_buffer_pop_front_to_fd_uint8 error while writing to "
           "file");
    exit(1);
  }
  circ_buf->size -= nWritten;
  if (circ_buf->size == 0) {
    circ_buf->iStart = 0;
    circ_buf->iStop = 0;
  } else {
    circ_buf->iStart = (circ_buf->iStart + nWritten) % circ_buf->capacity;
  }
  return nWritten;
}

void circular_buffer_clear_uint8(circular_buffer_uint8 *circ_buf) {
  circ_buf->size = 0;
  circ_buf->iStart = 0;
  circ_buf->iStop = 0;
}

size_t circular_buffer_push_back_string_uint8(circular_buffer_uint8 *circ_buf,
                                              const char *string) {
  size_t nPushed = 0;
  size_t iChar = 0;
  while (true) {
    const char *pChar = string + iChar;
    if ((*pChar) == '\0') {
      break;
    }
    circular_buffer_push_back_uint8(circ_buf, *pChar);
    iChar++;
  }
  return nPushed;
}
