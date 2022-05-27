
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
#ifndef STDINT_H
#define STDINT_H

// RPI-specific definitions only.
typedef unsigned int   uint32_t ;
typedef unsigned short uint16_t ;
typedef unsigned char  uint8_t ;
typedef int   int32_t ;
typedef short int16_t ;
typedef char  int8_t ;

// Definition needed by the printing routines.
// Number of bits in a char.
#define CHAR_BIT 8

// 64-bit signed integer
// Using these is possible, but much care 
// must be taken to ensure that they are 
// aligned on 64-bit (8 byte). Otherwise,
// results are incorrect.
typedef unsigned long long uint64_t ;
typedef long long int64_t ;

// The following definitions must have MAXINT
// and MAXUINT at the end, so that they don't
// interfere with the longer definitions of
// MAXINT64...
// 64-bit max unsigned int value
#define MAXUINT64 0xffffffffffffffff
// 64-bit signed limits
#define MAXINT64  0x7fffffffffffffff
#define MININT64  (-MAXINT64-1)
// 32-bit max and min signed int values
#define MAXINT  0x7fffffff
#define MININT  (-MAXINT-1)
// 32-bit max unsigned int value
#define MAXUINT 0xffffffff

#endif
