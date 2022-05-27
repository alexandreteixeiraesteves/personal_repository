
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
#ifndef SYSTEM_PARTITION_H
#define SYSTEM_PARTITION_H

#include <kernel/scheduler.h>

// Interface with the system partition (like all other
// partitions).
extern struct L1PartitionInterface system_partition_interface ;

// The high-level scheduler of the system partition
void system_partition_entry_point(void) ;

#endif
