
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
/* TIME constant and type definitions and management services     */
/*                                                                */
/*----------------------------------------------------------------*/
#ifndef APEX_TIME
#define APEX_TIME

#include <arinc653/types.h>

void TIMED_WAIT (/*in */ SYSTEM_TIME_TYPE DELAY_TIME,
		 /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void PERIODIC_WAIT (/*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void GET_TIME (/*out*/ SYSTEM_TIME_TYPE *SYSTEM_TIME,
	       /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void REPLENISH (/*in */ SYSTEM_TIME_TYPE BUDGET_TIME,
		/*out*/ RETURN_CODE_TYPE *RETURN_CODE );

#endif
