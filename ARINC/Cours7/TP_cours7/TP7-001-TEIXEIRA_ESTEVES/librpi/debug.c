
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
#include <libc/stdarg.h>    // For va_args
#include <libc/stdio.h>     // For sprintf_callable
#include <librpi/uart.h>    // For uart_puts
#include <librpi/debug.h>

/* Print a string, line by line. */
void debug_puts(const char* s) {
  while(*s) {
    debug_print_line_atomic(s) ;
    skip_line(s) ;
  }
}

#define DEBUG_BUFFER_SIZE 512

/* Debug printing function. */
void debug_printf(const char*fmt, ...) {
  // Store the buffer on the stack, so that 
  // it is not shared between concurrent 
  // pieces of code.
  //
  // The following code is quasi-identical 
  // with the one in sprintf. However, sharing
  // it is not possible, for it is impossible to
  // pass the arguments of an elliptic function
  // to another elliptif function. 
  char debug_buffer[DEBUG_BUFFER_SIZE] ;
  va_list va;
  va_start(va,fmt);
  vsnprintf(debug_buffer,DEBUG_BUFFER_SIZE,fmt,va);
  va_end(va);
  debug_puts(debug_buffer) ;
}

