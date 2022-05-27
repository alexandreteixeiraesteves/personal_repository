
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
/*--------------------------------------------------------------------*/
/*                                                                    */
/* QUEUING PORT constant and type definitions and management services */
/*                                                                    */
/*--------------------------------------------------------------------*/
#ifndef APEX_QUEUING
#define APEX_QUEUING

#include <arinc653/types.h>
#include <arinc653/process.h> // For WAITING_RANGE_TYPE

#define MAX_NUMBER_OF_QUEUING_PORTS SYSTEM_LIMIT_NUMBER_OF_QUEUING_PORTS

typedef NAME_TYPE         QUEUING_PORT_NAME_TYPE;
typedef APEX_INTEGER        QUEUING_PORT_ID_TYPE;

typedef struct {
  MESSAGE_RANGE_TYPE NB_MESSAGE;
  MESSAGE_RANGE_TYPE MAX_NB_MESSAGE;
  MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE;
  PORT_DIRECTION_TYPE PORT_DIRECTION;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} QUEUING_PORT_STATUS_TYPE;

void CREATE_QUEUING_PORT (/*in */ QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME,
			  /*in */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
			  /*in */ MESSAGE_RANGE_TYPE MAX_NB_MESSAGE,
			  /*in */ PORT_DIRECTION_TYPE PORT_DIRECTION,
			  /*in */ QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
			  /*out*/ QUEUING_PORT_ID_TYPE *QUEUING_PORT_ID,
			  /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void SEND_QUEUING_MESSAGE (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			   /*in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR, /* by reference */
			   /*in */ MESSAGE_SIZE_TYPE LENGTH,
			   /*in */ SYSTEM_TIME_TYPE TIME_OUT,
			   /*out*/ RETURN_CODE_TYPE *RETURN_CODE);

void RECEIVE_QUEUING_MESSAGE (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			      /*in */ SYSTEM_TIME_TYPE TIME_OUT,
			      /*out*/ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
			      /*out*/ MESSAGE_SIZE_TYPE *LENGTH,
			      /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void GET_QUEUING_PORT_ID (/*in */ QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME,
			  /*out*/ QUEUING_PORT_ID_TYPE *QUEUING_PORT_ID,
			  /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void GET_QUEUING_PORT_STATUS (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			      /*out*/ QUEUING_PORT_STATUS_TYPE *QUEUING_PORT_STATUS,
			      /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void CLEAR_QUEUING_PORT (/*in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
			 /*out*/ RETURN_CODE_TYPE *RETURN_CODE );
#endif
