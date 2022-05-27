
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
#include <libc/stdlib.h>         // For halt.
#include <librpi/system_timer.h> // For the SystemTimer
#include <librpi/debug.h>        // debug_printf
#include <kernel/init.h>         // For all init functions
#include <librpi/indent.h>


// This is the function that contains all system initialization,
// and should be called from the reset handler.
// initialization of the L1
// scheduler, including the reading of the configuration file, the
// reading of the partition ELF files, and the initialization of the
// scheduler and partition data structures, and the initialization of
// the memory isolation.
void kernel_init() {
  // Low-level initialization
  low_level_init() ;
  // Initialize the scheduler data structures and start the MMU.
  debug_printf("kernel_init: setup_default_page_tables_and_start_MMU.\n") ;
  setup_default_page_tables_and_start_MMU() ;
  // Load config from SD card and configure L1 scheduler
  debug_printf("kernel_init: load config and partition code from "
	       "SD card...\n") ;
  load_partitions() ;
  // Configuration is now fully completed. Print it.
  // The only thing to do is to enable preemptive scheduling.
  // Before doing this, porint the full configuration.
  debug_printf("kernel_init: configuration completed:\n") ;
  indent_increment_depth() ;
  print_full_kernel_config() ;
  indent_decrement_depth() ;
  // Enable preemptive scheduling by initializing the timer.
  debug_printf("kernel_init: configure and enable timer (init end)...\n") ;
  debug_printf("=====================================================\n") ;
  {
    // Enables everything, but does not change the CPU mode.
    SystemTimerStart(l1_scheduler_state.tick_length) ;
    // IRQs enabling in the ARM CPU (change CPRS)
    enable_interrupts() ;
  }
  halt() ;
}
