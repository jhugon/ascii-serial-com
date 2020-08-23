/**
 * \file
 * Functions and types for CRC checks.
 *
 * Generated on Sun Aug 23 10:49:33 2020
 * by pycrc v0.9.2, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x3d65
 *  - XorIn         = 0x0000
 *  - ReflectIn     = True
 *  - XorOut        = 0xffff
 *  - ReflectOut    = True
 *  - Algorithm     = table-driven
 */
#include "crc_tbl4bit.h" /* include the header file generated with pycrc */
#include <stdint.h>
#include <stdlib.h>

/**
 * Static table used for the table_driven implementation.
 */
static const crc_t crc_table[16] = {
    0x0000, 0xb26b, 0x29af, 0x9bc4, 0x535e, 0xe135, 0x7af1, 0xc89a,
    0xa6bc, 0x14d7, 0x8f13, 0x3d78, 0xf5e2, 0x4789, 0xdc4d, 0x6e26};

crc_t crc_update(crc_t crc, const void *data, size_t data_len) {
  const unsigned char *d = (const unsigned char *)data;
  unsigned int tbl_idx;

  while (data_len--) {
    tbl_idx = crc ^ *d;
    crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
    tbl_idx = crc ^ (*d >> 4);
    crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
    d++;
  }
  return crc & 0xffff;
}
