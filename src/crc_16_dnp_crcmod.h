#ifndef COMPUTE_CRC_H
#define COMPUTE_CRC_H

/** \file */

#include <stdint.h>
#include <stdlib.h>

// Automatically generated CRC function
// polynomial: 0x13D65, bit reverse algorithm
// From crcmod predefined CRC-16-DNP
uint16_t computeCRC_16_DNP(uint8_t *data, size_t len, uint16_t crc);

#endif
