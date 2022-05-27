
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
#include <arinc653/error.h>   //

void REPORT_APPLICATION_MESSAGE (/*in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
				 /*in */ MESSAGE_SIZE_TYPE LENGTH,
				 /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(MESSAGE_ADDR) ;
  UNUSED(LENGTH) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void CREATE_ERROR_HANDLER (/*in */ SYSTEM_ADDRESS_TYPE ENTRY_POINT,
			   /*in */ STACK_SIZE_TYPE STACK_SIZE,
			   /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(ENTRY_POINT) ;
  UNUSED(STACK_SIZE) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void GET_ERROR_STATUS (/*out*/ ERROR_STATUS_TYPE *ERROR_STATUS,
		       /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(ERROR_STATUS) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void RAISE_APPLICATION_ERROR (/*in */ ERROR_CODE_TYPE ERROR_CODE,
			      /*in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
			      /*in */ ERROR_MESSAGE_SIZE_TYPE LENGTH,
			      /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(ERROR_CODE) ;
  UNUSED(MESSAGE_ADDR) ;
  UNUSED(LENGTH) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

