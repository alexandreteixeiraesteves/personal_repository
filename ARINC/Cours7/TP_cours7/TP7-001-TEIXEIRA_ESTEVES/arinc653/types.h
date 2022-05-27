
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
/* This is a compilable ANSI C specification of the APEX interface,        */
/* derived from Section 3 of ARINC 653 Part 1.                             */
/* The declarations of the services given below are taken from the         */
/* standard, as are the enum types and the names of the others types.      */
/* Unless specified as implementation dependent, the values specified in   */
/* this appendix should be implemented as defined.                         */
/* This ANSI C specification follows the same structure (package) as       */
/* the Ada specification. The objective is to have the same definitions    */
/* and representations of the interface in both the Ada and C languages.   */
/* The two specifications are aligned in the same order, and define the    */
/* same items.                                                             */

/*----------------------------------------------------------------*/
/*                                                                */
/* Global constant and type definitions                           */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_TYPES
#define APEX_TYPES

#include <libc/stdint.h> // PLatform-dependent definitions

/*---------------------------*/
/* Domain limits             */
/*---------------------------*/
/* Implementation Dependent                                                */
/* These values define the domain limits and are implementation dependent. */
/* MODULE SCOPE */
/* JP: The maximal number of partitions must include the system
 partition and the application partitions.  I choose 14, which should
 allow me to use a single page table and only change memory domain
 permissions in the MMU. This means I can use 13 application
 partitions, which seems enough in practice. This is a true
 implementation limit. */
#define  SYSTEM_LIMIT_NUMBER_OF_PARTITIONS       14
#define  SYSTEM_LIMIT_NUMBER_OF_WINDOWS          64
#define  SYSTEM_LIMIT_NUMBER_OF_MESSAGES        128
#define  SYSTEM_LIMIT_MESSAGE_SIZE               64
/* PARTITION SCOPE -- I guess they can be different for each partition,
   but that I impose some global limits for simplicity. */
#define  SYSTEM_LIMIT_NUMBER_OF_PROCESSES        16
#define  SYSTEM_LIMIT_NUMBER_OF_QUEUING_PORTS    16


/*----------------------*/
/* Base APEX types      */
/*----------------------*/
/* Implementation Dependent                                        */
/* The actual size of these base types is system specific and the  */
/* sizes must match the sizes used by the implementation of the    */
/* underlying Operating System.                                    */
typedef uint8_t           APEX_BYTE;
typedef int32_t        APEX_INTEGER;
typedef uint32_t      APEX_UNSIGNED;
typedef int32_t   APEX_LONG_INTEGER;

/*----------------------*/
/* General APEX types   */
/*----------------------*/
typedef enum {
  NO_ERROR = 0,                   /* request valid and operation performed */
  NO_ACTION = 1,                 /* status of system unaffected by request */
  NOT_AVAILABLE = 2,           /* resource required by request unavailable */
  INVALID_PARAM = 3,             /* invalid parameter specified in request */
  INVALID_CONFIG = 4,         /* parameter incompatible with configuration */
  INVALID_MODE = 5,              /* request incompatible with current mode */
  TIMED_OUT = 6               /* time-out tied up with request has expired */
} RETURN_CODE_TYPE;

/* Names have fixed length, and all particular name types, defined
   later, are the same as this one */
#define MAX_NAME_LENGTH 30
typedef char    NAME_TYPE[MAX_NAME_LENGTH];

/* void pointer means that I should not be able to perform arithmetic
   on it. */
typedef void         * SYSTEM_ADDRESS_TYPE;
typedef APEX_BYTE      * MESSAGE_ADDR_TYPE;
/* message sizes, bounded by SYSTEM_LIMIT_MESSAGE_SIZE */
typedef APEX_INTEGER     MESSAGE_SIZE_TYPE;
/* message ranges, which must be bounded by 
   SYSTEM_LIMIT_NUMBER_OF_MESSAGES */
typedef APEX_INTEGER    MESSAGE_RANGE_TYPE;

typedef enum {
  SOURCE = 0,
  DESTINATION = 1
} PORT_DIRECTION_TYPE;

typedef enum {
  FIFO = 0,
  PRIORITY = 1
} QUEUING_DISCIPLINE_TYPE;

/* Time type: 32-bit signed integer with a 1 microsecond LSB */
typedef APEX_LONG_INTEGER SYSTEM_TIME_TYPE;
/* Infinite time is defined as any negative value 
   (-1 is used for the constant definition, however 
   all negative values should be treated as 
   INFINITE_TIME_VALUE. */
#define INFINITE_TIME_VALUE -1
/* JP: The following function is not part of the ARINC 653 spec. */
__attribute__((always_inline))
inline int32_t time_value_is_infinite(SYSTEM_TIME_TYPE s) {
  return s < 0 ;
}


#endif
