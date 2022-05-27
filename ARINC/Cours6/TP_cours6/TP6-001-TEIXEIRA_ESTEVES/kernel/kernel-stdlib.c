
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
#include <libc/stdlib.h>
#include <librpi/debug.h>

// Implementation of the kernel-specific routines defined
// in libc/stdlib.h and used everywhere in the kernel.

//
void fatal_error(const char *str) {
  debug_puts("FATAL: ") ;
  debug_puts(str) ;
  debug_puts("\n") ;
  exit(0) ;
}

//
void warning(const char* str) {
  debug_puts("WARNING: ") ;
  debug_puts(str) ;
  debug_puts("\n") ;
}

//
void halt() {
  for(;;) ;
}

/* This is NOT the regular POSIX routine because  */
/* it does not exit anything, simply              */
/* looping forever.                               */
/* It could be improved...                        */
void  exit(int status) {
  debug_printf("kernel_exit called with status %d...\n",status) ;
  halt() ;
}
