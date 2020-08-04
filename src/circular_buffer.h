#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef linux
#include <stdio.h>
#endif

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
 *  outfile is usually stderr or stdout
 *
 */
#ifdef linux
void circular_buffer_print_uint8(const circular_buffer_uint8 *circ_buf,
                                 FILE *outfile);
#endif

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
void circular_buffer_remove_front_to_uint8(circular_buffer_uint8 *circ_buf,
                                           const uint8_t value,
                                           const bool inclusive);

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
void circular_buffer_remove_back_to_uint8(circular_buffer_uint8 *circ_buf,
                                          const uint8_t value,
                                          const bool inclusive);

/** \brief circular buffer find index of first occurance of value
 *
 *  Finds the index of the first element with value equal to the given value.
 *  If the value is not found in the buffer, the return value will be >= the
 *  size of the buffer.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param value is the value you want to find
 *  \return the index where the value can be found using
 *     circular_buffer_get_element_uint8, will be >= size if not found
 *  \sa circular_buffer_get_element_uint8
 *
 */
size_t circular_buffer_find_first_uint8(const circular_buffer_uint8 *circ_buf,
                                        const uint8_t value);

/** \brief circular buffer find index of last occurance of value
 *
 *  Finds the index of the last element with value equal to the given value.
 *  If the value is not found in the buffer, the return value will be >= the
 *  size of the buffer.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param value is the value you want to find
 *  \return the index where the value can be found using
 *     circular_buffer_get_element_uint8, will be >= size if not found
 *  \sa circular_buffer_get_element_uint8
 *
 */
size_t circular_buffer_find_last_uint8(const circular_buffer_uint8 *circ_buf,
                                       const uint8_t value);

/** \brief circular buffer count the number of a given value
 *
 *  Counts the number of elements in the buffer that match the given value
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param value is the value you want to find
 *  \return the number of matches found
 *
 */
size_t circular_buffer_count_uint8(const circular_buffer_uint8 *circ_buf,
                                   const uint8_t value);

/** \brief circular buffer get first block
 *
 *  Internally, the data may wrap around the end of the block of memory. This
 * function gives you access to the memory block up to the end of the memory
 * block, or if the data doesn't wrap, up to the end of the data.
 *
 *  This can be used followed by circular_buffer_delete_first_block_uint8 to
 * "pop_front" a whole block of data at once.
 *
 *  \sa circular_buffer_delete_first_block_uint8
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \param outBlock is a pointer to the start of the block of memory
 *  \return size of the block pointed to by outBlock
 *
 */
size_t
circular_buffer_get_first_block_uint8(const circular_buffer_uint8 *circ_buf,
                                      const uint8_t **outBlock);

/** \brief circular buffer delete first block
 *
 *  Internally, the data may wrap around the end of the block of memory. This
 * function deletes the memory block from the current start of the circular
 * buffer up to the end of the memory block, or if the data doesn't wrap, up to
 * the end of the data.
 *
 *  This can be used after circular_buffer_get_first_block_uint8 to "pop_front"
 * a whole block of data at once.
 *
 *  \sa circular_buffer_get_first_block_uint8
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *  \return size of the block deleted
 *
 */
size_t
circular_buffer_delete_first_block_uint8(circular_buffer_uint8 *circ_buf);

/** \brief circular buffer push a block of memory on back
 *
 *  Pushes a block of memory onto back
 *
 *  Will wrap around and overrite front if buffer gets full
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 *  \param source is the start of the source memory block
 *
 *  \param source_size is the size of the source memory block
 *
 */
void circular_buffer_push_back_block_uint8(circular_buffer_uint8 *circ_buf,
                                           const uint8_t *source,
                                           size_t source_size);

/** \brief circular buffer pop from front into a block of memory
 *
 *  Pops elements from front into a block of memory
 *
 *  Won't pop all elements if circular buffer wraps around it's internal
 *  memory or there isn't enough source size
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 *  \param destination is the start of the destination memory block
 *
 *  \param source_size is the size of the destination memory block
 *
 *  \return total number of elements popped from buffer (and written to block)
 *
 */
size_t circular_buffer_pop_front_block_uint8(circular_buffer_uint8 *circ_buf,
                                             uint8_t *destination,
                                             size_t dest_size);

/** \brief circular buffer push back reading from file descriptor
 *
 *  Calls POSIX read once into the circular buffer
 *
 *  Doesn't overwrite the front of the buffer like the normal push_back
 *  does. Stops when full.
 *
 *  Won't necessarily fill up the buffer, even if there are bytes to read
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 *  \return total number of elements pushed onto buffer (and read from file)
 *
 */
#ifdef linux
size_t circular_buffer_push_back_from_fd_uint8(circular_buffer_uint8 *circ_buf,
                                               const int fd);

/** \brief circular buffer pop front writing to file descriptor
 *
 *  Calls POSIX write once from the circular buffer
 *
 *  Won't necessarily empty the buffer, even if the file is available for
 *  writing
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 *  \return total number of elements popped from buffer (and written to file)
 *
 */
size_t circular_buffer_pop_front_to_fd_uint8(circular_buffer_uint8 *circ_buf,
                                             const int fd);
#endif

/** \brief circular buffer clear
 *
 *  Resets the circular buffer to empty and starting from the beginning of the
 * memory buffer
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 */
void circular_buffer_clear_uint8(circular_buffer_uint8 *circ_buf);

/** \brief circular buffer push back string (null terminated)
 *
 *  Pushes null terminated string onto buffer (without pushing the null)
 *
 *  For testing, NOT RECOMMENDED FOR MICROCONTROLLER USE
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 *  \param string: null terminated string like a string literal
 *
 *  \return total number of elements pushed onto buffer
 *
 */
size_t circular_buffer_push_back_string_uint8(circular_buffer_uint8 *circ_buf,
                                              const char *string);

/** \brief circular buffer remove unfinished frames from front of buffer
 *
 *  Removes from the front of the buffer any frames that haven't ended before
 *  an ended frame. If the buffer just contains an unfinished frame (e.g. that
 *  hasn't finished reading in yet), that isn't removed.
 *
 *  More concretely: The first endChar is found. Any elements before the
 *  startChar preceding endChar are removed. If the endChar isn't found, any
 *  elements before the last startChar are removed. If no startChar is found
 *  either, the buffer is cleared.
 *
 *  Additionally, if endChar is front or there is no startChar before the first
 *  endChar, everything up to and including endChar is deleted and the function
 *  is recursed.
 *
 *  \param circ_buf is a pointer to an initialized circular buffer struct
 *
 *  \param startChar designates the start of frames
 *
 *  \param endChar designates the end of frames
 *
 *  \return the number of elements removed
 *
 */
size_t circular_buffer_remove_front_unfinished_frames_uint8(
    circular_buffer_uint8 *circ_buf, const char startChar, const char endChar);

#endif
