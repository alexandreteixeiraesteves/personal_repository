
/*  Copyright (C) 2016, Inria
    Author: Dumitru Potop-Butucaru.

    This file is part of RPi653. 

    RPi653 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    RPi653 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UTIL_H
#define UTIL_H

#include <libc/stdint.h>
#include <libc/stddef.h>

// Support for unaligned data access
static inline void write_word(uint32_t val, uint8_t *buf, int offset) {
  buf[offset + 0] = val & 0xff;
  buf[offset + 1] = (val >> 8) & 0xff;
  buf[offset + 2] = (val >> 16) & 0xff;
  buf[offset + 3] = (val >> 24) & 0xff;
}

static inline void write_halfword(uint16_t val, uint8_t *buf, int offset) {
  buf[offset + 0] = val & 0xff;
  buf[offset + 1] = (val >> 8) & 0xff;
}

static inline void write_byte(uint8_t byte, uint8_t *buf, int offset) {
  buf[offset] = byte;
}

static inline uint32_t read_word(uint8_t *buf, int offset) {
  uint32_t result = buf[offset+3] ;
  result = (result<<8) + buf[offset+2] ;
  result = (result<<8) + buf[offset+1] ;
  result = (result<<8) + buf[offset] ;
  return result ;
  
  /*  uint32_t b0 = buf[offset + 0] & 0xff;
  uint32_t b1 = buf[offset + 1] & 0xff;
  uint32_t b2 = buf[offset + 2] & 0xff;
  uint32_t b3 = buf[offset + 3] & 0xff;
  
  return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
  */
}

static inline uint16_t read_halfword(uint8_t *buf, int offset) {
  uint16_t result = buf[offset+1] ;
  result = (result<<8) + buf[offset] ;
  return result ;
  /*
  uint16_t b0 = buf[offset + 0] & 0xff;
  uint16_t b1 = buf[offset + 1] & 0xff;
  
  return b0 | (b1 << 8);*/
}

static inline uint8_t read_byte(uint8_t *buf, int offset) {
  return buf[offset];
}

static inline uint32_t byte_swap(uint32_t in) {
  uint32_t b0 = in & 0xff;
  uint32_t b1 = (in >> 8) & 0xff;
  uint32_t b2 = (in >> 16) & 0xff;
  uint32_t b3 = (in >> 24) & 0xff;
  uint32_t ret = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
  return ret;
}

static inline uint32_t increment_modulo(uint32_t num, uint32_t base) {
  num += 1 ;
  if(num >= base) num = 0 ;
  return num ;
}

#endif

