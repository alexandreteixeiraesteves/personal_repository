
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
#ifndef STRING_H
#define STRING_H

#include <libc/stddef.h> // For size_t

// This function is more important than the ones
// following because it is called by the kernel 
// very early in the boot process to set .bbs 
// to 0.
//
// This function only requires that a valid stack 
// has been set.
void   bzero(void *s, size_t n);

// These other functions are not called as early in 
// the boot process and may benefit from many
// debugging facilities not available in bzero.
void*  memset(void *b, int c, size_t len);
size_t strlen(const char *s);
char*  strcpy(char *s1, const char *s2);
char*  strncpy(char *restrict s1, const char *restrict s2, 
	       size_t n);
size_t strlcpy(char *restrict s1, const char *restrict s2, 
	       size_t n) ;
int    strcmp(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
void*  memcpy(void *restrict s1, const void *restrict s2, size_t n);



#endif
