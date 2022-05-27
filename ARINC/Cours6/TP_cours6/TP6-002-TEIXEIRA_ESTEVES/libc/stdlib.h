
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
#ifndef STDLIB_H
#define STDLIB_H

#include <libc/stddef.h>

// The difficult to implement (correctly) malloc and
// free, plus an initialization function for the heap
// memory and a debug function for printing the whole
// heap and which does not use malloc itself.
void free(void *ptr) ;
void* malloc(size_t size);
int32_t posix_memalign(void **memptr, 
		       size_t alignment, size_t size);
void init_malloc(char* base_address, uint32_t size) ;
void DebugPrintHeapOrganization(void) ;

// Useful macro, which forces rapid errors instead of
// (certain) heisenbugs.
#define safe_free(ptr) free(ptr);ptr=NULL;


// External definitions. Must be implemented by the platform.
extern void fatal_error(const char*) ;
extern void warning(const char*) ;
extern void exit(int32_t) ;
extern void halt(void) ;

#endif
