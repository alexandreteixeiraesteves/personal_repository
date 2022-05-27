
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
/* PARTITION constant and type definitions and management services*/
/*                                                                */
/*----------------------------------------------------------------*/
#ifndef APEX_PARTITION
#define APEX_PARTITION

#include <arinc653/process.h> // For LOCK_LEVEL_TYPE and other types
#include <arinc653/types.h>

#define MAX_NUMBER_OF_PARTITIONS SYSTEM_LIMIT_NUMBER_OF_PARTITIONS

typedef enum {
  IDLE       = 0,
  COLD_START = 1,
  WARM_START = 2,
  NORMAL     = 3,
} OPERATING_MODE_TYPE;

typedef APEX_INTEGER PARTITION_ID_TYPE;

typedef enum {
  NORMAL_START         = 0, /* normal power-up */
  PARTITION_RESTART    = 1, /* either due to COLD_START or WARM_START by
			       the partition itself, through the
			       SET_PARTITION_MODE service */
  HM_MODULE_RESTART    = 2, /* recovery action taken at module level by 
			       the HM */
  HM_PARTITION_RESTART = 3  /* recovery action taken at partition level 
			       by the HM */
} START_CONDITION_TYPE;

typedef struct {
  SYSTEM_TIME_TYPE     PERIOD;
  SYSTEM_TIME_TYPE     DURATION;
  PARTITION_ID_TYPE    IDENTIFIER;
  LOCK_LEVEL_TYPE      LOCK_LEVEL;
  OPERATING_MODE_TYPE  OPERATING_MODE;
  START_CONDITION_TYPE START_CONDITION;
} PARTITION_STATUS_TYPE;

void GET_PARTITION_STATUS (/*out*/ PARTITION_STATUS_TYPE *PARTITION_STATUS,
			   /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

void SET_PARTITION_MODE (/*in */ OPERATING_MODE_TYPE OPERATING_MODE,
			 /*out*/ RETURN_CODE_TYPE *RETURN_CODE );

#endif
