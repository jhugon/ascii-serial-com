#include "circular_buffer.h"
#include <assert.h>

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

void circular_buffer_print_uint8(const circular_buffer_uint8 *circ_buf) {

  //////typedef struct circular_buffer_uint8_struct {
  //////  size_t capacity; /**< capacity of actual data buffer */
  //////  size_t size;     /**< N elements in circ buffer */
  //////  size_t iStart;   /**< front element of buffer */
  //////  size_t iStop;    /**< 1 past the back element of buffer */
  //////  uint8_t *buffer; /**< pointer to actual data buffer */
  //////} circular_buffer_uint8;

  printf("circular_buffer_uint8, capacity: %zu\n", circ_buf->capacity);
  printf("  size: %zu iStart: %zu iStop %zu\n", circ_buf->size,
         circ_buf->iStart, circ_buf->iStop);
  printf("  Content: [ ");
  for (size_t i = 0; i < circ_buf->size; i++) {
    printf("%" PRIu8 " ", circular_buffer_get_element_uint8(circ_buf, i));
  }
  printf("]\n");
  printf("  Raw Memory: [ ");
  for (size_t i = 0; i < circ_buf->capacity; i++) {
    printf("%" PRIu8 " ", *(circ_buf->buffer + i));
  }
  printf("]\n");
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
