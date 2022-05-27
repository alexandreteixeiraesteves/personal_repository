
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
#include <libc/stddef.h>      // For UNUSED
#include <arinc653/types.h>   // For the return code and for basic types
#include <arinc653/process.h> //

void SET_PRIORITY (/*in */ PROCESS_ID_TYPE PROCESS_ID,
		   /*in */ PRIORITY_TYPE PRIO,
		   /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(PROCESS_ID) ;
  UNUSED(PRIO) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}


void LOCK_PREEMPTION (/*out*/ LOCK_LEVEL_TYPE *LOCK_LEVEL,
		      /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(LOCK_LEVEL) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void UNLOCK_PREEMPTION (/*out*/ LOCK_LEVEL_TYPE *LOCK_LEVEL,
			/*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(LOCK_LEVEL) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void GET_PROCESS_ID (/*in */ PROCESS_NAME_TYPE PROCESS_NAME,
		     /*out*/ PROCESS_ID_TYPE *PROCESS_ID,
		     /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(PROCESS_NAME) ;
  UNUSED(PROCESS_ID) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}
