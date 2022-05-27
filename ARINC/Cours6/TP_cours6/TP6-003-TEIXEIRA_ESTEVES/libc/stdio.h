
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
#ifndef STDIO_H
#define STDIO_H

#include <libc/stddef.h> // For size_t
#include <libc/stdarg.h> // For the defintion for va_list

/* Simple implementations of sprintf and vsprintf that only 
   cover the formats %u %d %x/%X %s. The only supported
   qualifiers are # and a number between 1 and 9 which
   give the min size of the field. */
int snprintf(char* restrict buffer,
	    size_t buf_size,
	    const char * restrict fmt, ...) ;
int vsnprintf(char* restrict result,
	      size_t buf_size,
	      const char * restrict fmt,
	      va_list va) ;

/* Auxiliary functions used by debug functions that
   avoid the high-level debug_printf. These functions 
   do not use malloc/free. */
/* Convert an unsigned int to ascii form. Takes as argument 
   the number, the translation base (works well for 
   2, 8, 10, 16, but should work for the other, too,
   provided that the size of the base does not go beyond
   the characters in ascii), the output buffer (which must 
   be provided), and the minimal length of the 
   number, needed to create numbers of specific
   sizes (like %8x in printf). */
int uint32ascii(uint32_t     num,       /* Number to print. */
		unsigned int base,      /* Base. */
		unsigned int min_length,/* Min number of digits. */
		unsigned int max_length,/* Max number of digits.
					   Minimum 1 for positive,
					   2 for negative. */
		char * out_buffer);     /* Out buffer. Attention,
					   it must have at least
					   max_length+1 bytes. */
/* Same for an integer. */
int int2ascii (int32_t      num,       /* Number to print. */
	       unsigned int base,      /* Base. */
	       unsigned int min_length,/* Min number of digits. */
	       unsigned int max_length,/* Max number of digits.
					  Minimum 1 for positive,
					  2 for negative. */
	       char * out_buffer);     /* Out buffer. Attention,
					  it must have at least
					  max_length+1 bytes. */
/* Simple sscanf functions. */
int sscanf(const char * restrict s, 
	   const char * restrict format, ...);
int vsscanf(const char * restrict s, 
	   const char * restrict format, va_list va);

#endif
