
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
#include <libc/stdlib.h>                 // For fatal_error
#include <libc/stddef.h>                 // For UNUSED
#include <librpi/debug.h>                // For debug_printf
#include <libpartition/scheduler.h>      // For data structures.
#include <libpartition/svc-call.h>       // For svc calls
#include <arinc653/time.h>


void TIMED_WAIT (/*in */ SYSTEM_TIME_TYPE DELAY_TIME,
		 /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  //debug_printf("TIMED_WAIT called.\n") ;
  switch(l1_partition_interface.partition_status.OPERATING_MODE) {
  case NORMAL:
    {
      if(DELAY_TIME % l1_partition_interface.tick_length) {
	DELAY_TIME = (DELAY_TIME/l1_partition_interface.tick_length)+1 ;
      } else {
	DELAY_TIME = DELAY_TIME/l1_partition_interface.tick_length ;
      } 
      *RETURN_CODE = svc_l2_request_timed_wait(DELAY_TIME) ;
    }
    break ;
  default:
    debug_printf("Calling TIMED_WAIT in non-NORMAL mode.\n") ;
    *RETURN_CODE = INVALID_MODE ;
    break ;
  }
  //  debug_printf("TIMED_WAIT finished.\n") ;
}

void GET_TIME (/*out*/ SYSTEM_TIME_TYPE *SYSTEM_TIME,
	       /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  *RETURN_CODE = svc_a653_get_time(SYSTEM_TIME) ;  
}
