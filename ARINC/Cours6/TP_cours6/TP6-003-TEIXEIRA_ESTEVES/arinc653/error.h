
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
/*----------------------------------------------------------------*/
/*                                                                */
/* ERROR constant and type definitions and management services    */
/*                                                                */
/*----------------------------------------------------------------*/
#ifndef APEX_ERROR
#define APEX_ERROR

#include <arinc653/types.h>
#include <arinc653/process.h>

#define MAX_ERROR_MESSAGE_SIZE 128

typedef APEX_INTEGER                      ERROR_MESSAGE_SIZE_TYPE;
typedef APEX_BYTE      ERROR_MESSAGE_TYPE[MAX_ERROR_MESSAGE_SIZE];

typedef enum {
  DEADLINE_MISSED   = 0,
  APPLICATION_ERROR = 1,
  NUMERIC_ERROR     = 2,
  ILLEGAL_REQUEST   = 3,
  STACK_OVERFLOW    = 4,
  MEMORY_VIOLATION  = 5,
  HARDWARE_FAULT    = 6,
  POWER_FAIL        = 7
} ERROR_CODE_TYPE;

typedef struct {
  ERROR_CODE_TYPE         ERROR_CODE;
  ERROR_MESSAGE_SIZE_TYPE LENGTH;
  PROCESS_ID_TYPE         FAILED_PROCESS_ID;
  SYSTEM_ADDRESS_TYPE     FAILED_ADDRESS;
  ERROR_MESSAGE_TYPE      MESSAGE;
} ERROR_STATUS_TYPE;

void REPORT_APPLICATION_MESSAGE (/*in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
				 /*in */ MESSAGE_SIZE_TYPE LENGTH,
				 /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void CREATE_ERROR_HANDLER (/*in */ SYSTEM_ADDRESS_TYPE ENTRY_POINT,
			   /*in */ STACK_SIZE_TYPE STACK_SIZE,
			   /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void GET_ERROR_STATUS (/*out*/ ERROR_STATUS_TYPE *ERROR_STATUS,
		       /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void RAISE_APPLICATION_ERROR (/*in */ ERROR_CODE_TYPE ERROR_CODE,
			      /*in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
			      /*in */ ERROR_MESSAGE_SIZE_TYPE LENGTH,
			      /*out*/ RETURN_CODE_TYPE *RETURN_CODE );
#endif

