
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
#include <librpi/debug.h>             // For debug_printf
#include <kernel/scheduler.h>         // For the type of the L2 scheduler
#include <kernel/system-partition.h>  // For the function prototypes
#include <kernel/system-partition-console.h> // For the console driver

// System partition interface, same data structure as for all
// partitions.
struct L1PartitionInterface system_partition_interface ;

void system_partition_init() {
  // Failing to set this value will lead to the
  // partition being restarted indefinitely.
  system_partition_interface.is_initialized = 1 ;
  
  // Now, initialize the drivers.
  console_init() ;
}

// The system partition runs kernel driver code that accesses
// shared resources such as the screen or the network.
// When creating drivers to add here, follow the example of the
// screen driver. 
// ATTENTION: before adding here any sort of code that uses
// any form of malloc/free, I should make malloc and free
// interruptible.
// ATTENTION2: the code here should run in SYSTEM mode, so that
// the registers are not banked. Otherwise, the context saving
// code in the interrupt handlers will not be correct.
void system_partition_entry_point() {
  //
  debug_printf("system_partition_entry_point: entered.\n") ;
  // Init
  system_partition_init() ;
  //
  debug_printf("system_partition_entry_point: init completed.\n") ;
  // Main loop
  for(;;) {
    console_driver() ;
  }
}


