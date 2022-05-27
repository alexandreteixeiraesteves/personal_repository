
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
#ifndef DEBUG_H
#define DEBUG_H

#include <librpi/uart.h>   // For uart_puts.
#include <libc/stddef.h>   // For NULL.

/* Console print routines used for debug purposes that can be
   deactivated if necessary. I should actually, at some point,
   introduce a debug level allowing to gradually allow messages.
*/
void debug_printf(const char *fmt, ...);
/* Print a string to the UART one character at a time. */
void debug_puts(const char *str);
/* Function that is used in debug.c, but also during system 
 * load. This function skips one line from the input
 * string. Used with debug_print_line_atomic to 
 * print with atomic lines. It either returns a 
 * pointer to the next line, or to an empty string.
 * For some reason, it does not accept the header 
 * const char* skip_line(const char* s)
 * because it says that "x = skip_line(x)" infringes on
 * the const qualifier. It may be that it infringes on 
 * the restrict qualifier from the definition of
 * sscanf (cf. init-load-partitions.c), but in any case 
 * I'm putting it into a macro.
 */
#define skip_line(s) \
  { \
    while((*s!='\n')&&(*s!='\r')&&(*s!='\0')) { \
      s++ ; \
    } \
    if(*s != '\0') s++ ; \
  }



/* Extern function that is specific to the caller context
   (kernel or partition). It is called by debug_puts and 
   debug_printf. */
extern void debug_print_line_atomic(const char* str) ;

#endif
