#ifndef AVR_TIMERS_H
#define AVR_TIMERS_H

/** \file */

#include <avr/io.h>
#include <stdint.h>

/** \brief Initialize Timer0
 *
 * Normal waveform gen mode (WGM2:1 are 0)
 *
 * \param cs 3 bits: 0: no timer, 1: no prescale, 2, /8 prescale
 *      3: /64 prescale 4: /256 prescale 5: /1024 prescale
 *
 * \param ctc (1 bit); 1: clear counter when counter reaches OCR0A;
 *      0: clear counter when read 255
 *
 * \param timsk0: bits enable interrupts:
 *      bit 0: interrup on overflow
 *      bit 1: interrupt on match OCR0A
 *      bit 2: interrupt on match OCR0B
 *
 * OCR0A and OCR0B are writable compare registers to set when
 *      interrupt and ctc happens
 *
 */
#define TIMER0_Init(cs, ctc, timsk0)                                           \
  TCCR0A = ((ctc & 1) << WGM01);                                               \
  TCCR0B = cs & 0b111;                                                         \
  TIMSK0 = timsk0 & 0b111;

#endif
