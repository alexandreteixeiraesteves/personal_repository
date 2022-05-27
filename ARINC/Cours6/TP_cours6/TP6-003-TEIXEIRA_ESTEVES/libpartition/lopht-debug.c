
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
#include <libpartition/lopht-debug.h>
#include <libpartition/error.h>
#include <librpi/debug.h>
#include <libc/stdio.h>

#define PRINT_BUF_SIZE 256
char print_buf[PRINT_BUF_SIZE] ;

void console_perror(const RETURN_CODE_TYPE rc,
		    const char* partition_name,
		    const char* op_name) {
  if ( rc != NO_ERROR ) {
    snprintf(print_buf,
	     PRINT_BUF_SIZE,
	     "%s: %s: %s.",
	     partition_name,
	     op_name,
	     error2string(rc) );
    debug_puts(print_buf) ;
  }
}
