
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
#ifndef MMAP_C_H
#define MMAP_C_H

#include <libc/stdint.h>

// These constants are defined in my loader script
// and can be used in the code to identify segment
// limits.
// 
// Attention: _text_start does not have as value
// the address of the .text start. Instead, getting
// the start of .text is done by using the address
// &_text_start.
extern const uint32_t _text_start ;
extern const uint32_t _text_end ;
extern const uint32_t _rodata_start ;
extern const uint32_t _rodata_end ;
extern const uint32_t _data_start ;
extern const uint32_t _data_end ;
extern const uint32_t _bss_start ;
extern const uint32_t _bss_end ;
extern const uint32_t _end ;

#endif
