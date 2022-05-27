
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
#include <libc/stdlib.h>             // For halt, fatal_error
#include <librpi/debug.h>            // For debug functions
#include <libpartition/scheduler.h>  // For scheduler state

//==========================================================
// Scheduler state
//==========================================================
// The interface with the L1 scheduler goes at the beginning
// of the .bss, so that it's identifiable by the loader of
// the kernel.
__attribute__((section(".bss.interface")))
struct L1PartitionInterface l1_partition_interface ;
// This is the actual L2 scheduler state (the local part,
// encluding the elements already in the interface.
// Note it's in .bss, too, so it is zeroed.
struct L2SchedulerState l2_scheduler_state ;

//==========================================================
// Step function of the scheduler, called after init
// whenever I don't return directly to the caller.
//==========================================================
__attribute__((section(".text.l2entrypoint")))
void L2_entry_point() {
  //debug_puts("L2_entry_point: just entered.\n") ;
  // This is the scheduler of the partition. It is a function that
  // never ends.
  switch(l1_partition_interface.partition_status.OPERATING_MODE) {
  case IDLE:
    // Do nothing, forever.
    halt() ;
    
  case COLD_START:
  case WARM_START:
    // Start the partition. Non-terminating function that either
    // enter another mode, or runs forever.
    L2_start_mode() ;
    
  case NORMAL:
    // Scheduler code. Non-terminating function that either
    // enter another mode, or runs forever.
    L2_normal_mode_scheduler() ;
    
  default:
    // Just in case there are weird errors.
    fatal_error("L2_entry_point:Non-existant operating mode.\n") ;
  }
}

