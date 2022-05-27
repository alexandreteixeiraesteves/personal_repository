
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
#include <arinc653/queuing.h> //
#include <kernel/scheduler.h>
#include <libpartition/svc-call.h>

QUEUING_PORT_ID_TYPE port_counter = 0 ;
void CREATE_QUEUING_PORT (/*in */ QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME,
			  /*in */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
			  /*in */ MESSAGE_RANGE_TYPE MAX_NB_MESSAGE,
			  /*in */ PORT_DIRECTION_TYPE PORT_DIRECTION,
			  /*in */ QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
			  /*out*/ QUEUING_PORT_ID_TYPE *QUEUING_PORT_ID,
			  /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(QUEUING_PORT_NAME) ;
  UNUSED(MAX_MESSAGE_SIZE) ;
  UNUSED(MAX_NB_MESSAGE) ;
  UNUSED(PORT_DIRECTION) ;
  UNUSED(QUEUING_DISCIPLINE) ;
  UNUSED(QUEUING_PORT_ID) ;
  *QUEUING_PORT_ID = port_counter ;
  port_counter ++ ;
  *RETURN_CODE = NO_ERROR ;
}

void SEND_QUEUING_MESSAGE (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			   /*in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR, /* by reference */
			   /*in */ MESSAGE_SIZE_TYPE LENGTH,
			   /*in */ SYSTEM_TIME_TYPE TIME_OUT,
			   /*out*/ RETURN_CODE_TYPE *RETURN_CODE) {
  UNUSED(TIME_OUT) ;
  struct QueuingMessage qm ;
  qm.size = LENGTH ;
  qm.data = MESSAGE_ADDR ;
  *RETURN_CODE = svc_send_queuing(QUEUING_PORT_ID,
				  &qm) ;

}

void RECEIVE_QUEUING_MESSAGE (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			      /*in */ SYSTEM_TIME_TYPE TIME_OUT,
			      /*out*/ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
			      /*out*/ MESSAGE_SIZE_TYPE *LENGTH,
			      /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(TIME_OUT) ;
  struct QueuingMessage qm ;
  qm.data = MESSAGE_ADDR ;
  *RETURN_CODE = svc_recv_queuing(QUEUING_PORT_ID,
				  &qm) ;
  *LENGTH = qm.size ;
}

void GET_QUEUING_PORT_ID (/*in */ QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME,
			  /*out*/ QUEUING_PORT_ID_TYPE *QUEUING_PORT_ID,
			  /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(QUEUING_PORT_NAME) ;
  UNUSED(QUEUING_PORT_ID) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void GET_QUEUING_PORT_STATUS (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			      /*out*/ QUEUING_PORT_STATUS_TYPE *QUEUING_PORT_STATUS,
			      /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(QUEUING_PORT_ID) ;
  UNUSED(QUEUING_PORT_STATUS) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}

void CLEAR_QUEUING_PORT (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			 /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  UNUSED(QUEUING_PORT_ID) ;
  *RETURN_CODE = NOT_AVAILABLE ;
}


