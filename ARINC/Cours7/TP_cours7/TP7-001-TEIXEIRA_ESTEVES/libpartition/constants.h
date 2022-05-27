
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
#ifndef CONSTANTS_H
#define CONSTANTS_H


#include <arinc653/types.h> // For SYSTEM_LIMIT_...

//==========================================================
// Partition-specific constants
//==========================================================
//
// Memory handling stuff -- partition-specific.
// Reserve this for the (unique) heap of the partition.
// Must be a multiple of 16 (4 words)
#define PARTITION_HEAP_SIZE       0x10000
// Stack shared between the initialization code (including
// elaboration code) and the L2 scheduler. Must be a multiple
// of 4 (word-aligned).
#define PARTITION_L2_STACK_SIZE   0x2000
// The interface with the L1 scheduler.
#define MIN_PROCESS_STACK_SIZE    0x100
//
// The number of processes is the imoplem. limit.
#define MAX_PROCESS_NUMBER SYSTEM_LIMIT_NUMBER_OF_PROCESSES


#endif
