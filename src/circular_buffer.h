#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/** \brief circular buffer struct
 *
 *  Keeps track of the state of the circular buffer
 *  Use this for a buffer of unit8's.
 *
 */
typedef struct circular_buffer_uint8_struct {
  size_t capacity; /**< capacity of actual data buffer */
  size_t size;     /**< N elements in circ buffer */
  size_t iStart;   /**< front element of buffer */
  size_t iStop;    /**< 1 past the back element of buffer */
  uint8_t *buffer; /**< pointer to actual data buffer */
} circular_buffer_uint8;

/** \brief circular buffer init method
 *
 *  Initialize circular buffer
 *  THE BUFFER MUST BE SMALLER THAN (< not <=) SIZE_T ON THE PLATFORM
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an uninitialized circular buffer struct
 *  \param capacity is the length of buffer (uint8_t array)
 *  \param buffer is a pointer to the buffer (uint8_t array), which must be
 * pre-allocated
 *
 */
void circular_buffer_init_uint8(circular_buffer_uint8 *circ_buf,
                                const size_t capacity, uint8_t *buffer);

/** \brief circular buffer get number of elements
 *
 *  circular buffer get number of elements
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \return the number of elements in the buffer
 *
 */
size_t circular_buffer_get_size_uint8(const circular_buffer_uint8 *circ_buf);

/** \brief circular buffer get if full
 *
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \return true true if the buffer is full
 *
 */
bool circular_buffer_is_full_uint8(const circular_buffer_uint8 *circ_buf);

/** \brief circular buffer get if empty
 *
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \return true if the buffer is empty
 *
 */
bool circular_buffer_is_empty_uint8(const circular_buffer_uint8 *circ_buf);

/** \brief circular buffer print contents
 *
 *  Use this for debugging
 *
 */
void circular_buffer_print_uint8(const circular_buffer_uint8 *circ_buf);

/** \brief circular buffer get element
 *
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param iElement indexes the circular buffer. The element will be iElements
 * back from the front of the circular buffer \return the entry at iElement
 *
 */
uint8_t circular_buffer_get_element_uint8(const circular_buffer_uint8 *circ_buf,
                                          const size_t iElement);

/** \brief circular buffer push front
 *
 *  Pushes the given element onto the front of the buffer, making it the new
 * front element. Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param element the element to be pushed onto the front of the buffer
 *
 */
void circular_buffer_push_front_uint8(circular_buffer_uint8 *circ_buf,
                                      const uint8_t element);

/** \brief circular buffer push back
 *
 *  Pushes the given element onto the back of the buffer, making it the new back
 * element. Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param element the element to be pushed onto the back of the buffer
 *
 */
void circular_buffer_push_back_uint8(circular_buffer_uint8 *circ_buf,
                                     const uint8_t element);

/** \brief circular buffer pop front
 *
 *  Pops an element off of the front of the buffer, both removing it and
 * returning it. User's responsibility to first check if the buffer is empty.
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \return element the element popped off the front of the buffer
 *
 */
uint8_t circular_buffer_pop_front_uint8(circular_buffer_uint8 *circ_buf);

/** \brief circular buffer pop back
 *
 *  Pops an element off of the back of the buffer, both removing it and
 * returning it. User's responsibility to first check if the buffer is empty.
 *  Use this for a buffer of unit8's.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \return the element to be popped off the back of the buffer
 *
 */
uint8_t circular_buffer_pop_back_uint8(circular_buffer_uint8 *circ_buf);

/** \brief circular buffer remove elements from front until you find the given
 * value
 *
 *  Remove elements from the front of the buffer until the given value is found.
 *  If the value isn't in the buffer, will empty the buffer.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param value is the value you want to remove the front of the buffer up to.
 *  \param inclusive: if true, then remove the given value, otherwise all before
 * the given value
 *
 */
// void circular_buffer_remove_front_to_uint8(circular_buffer_uint8 *circ_buf,
// const uint8_t value, const bool inclusive);

/** \brief circular buffer remove elements from back until you find the given
 * value
 *
 *  Remove elements from the back of the buffer until the given value is found.
 *  If the value isn't in the buffer, will empty the buffer.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param value is the value you want to remove the back of the buffer up to.
 *  \param inclusive: if true, then remove the given value, otherwise all after
 * the given value
 *
 */
// void circular_buffer_remove_back_to_uint8(circular_buffer_uint8 *circ_buf,
// const uint8_t value, const bool inclusive);

/** \brief circular buffer find index of first occurance of value
 *
 *  Finds the index of the first element with value equal to the given value.
 *  If the value is not found in the buffer, the return value will be >= the
 *  size of the buffer.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param value is the value you want to find
 *  \return the index where the value can be found using
 * circular_buffer_get_element_uint8, will be >= size if not found \sa
 * circular_buffer_get_element_uint8
 *
 */
size_t circular_buffer_find_first_uint8(const circular_buffer_uint8 *circ_buf,
                                        const uint8_t value);

#endif
