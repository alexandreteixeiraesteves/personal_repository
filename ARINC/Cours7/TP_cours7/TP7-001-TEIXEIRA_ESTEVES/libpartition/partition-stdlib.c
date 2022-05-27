
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
#include <libc/stdlib.h>                  // For the prototypes
#include <librpi/debug.h>                 // For debug printing
#include <libpartition/svc-l2-request.h>  // For the various structs and
                                          // constants.
#include <libpartition/svc-call.h>        // For svc_sched_exit 
#include <libpartition/scheduler.h>       // For the scheduler data struct.

// Implementation of the kernel-specific routines defined
// in libc/stdlib.h and used everywhere in the kernel.

//
void fatal_error(const char *str) {
  RETURN_CODE_TYPE rc ;
  debug_printf("FATAL: %s.\n",str) ;
  SET_PARTITION_MODE(IDLE,&rc) ;
}

//
void warning(const char* str) {
  debug_printf("WARNING: %s.\n",str) ;
}

//
void halt() {
  for(;;) ;
}

/* This function should never be called in a partition. 
 * ARINC 653 has its own error handling routines.
 */
void  exit(int status) {
  UNUSED(status) ;
  fatal_error("exit() should never be called inside a partition.") ;
}
