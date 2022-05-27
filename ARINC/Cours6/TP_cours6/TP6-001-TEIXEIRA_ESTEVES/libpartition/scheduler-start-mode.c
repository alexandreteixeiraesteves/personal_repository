
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
#include <libc/stdlib.h>            // For init_malloc
#include <libc/string.h>            // For bzero
#include <librpi/mmap_c.h>          // For _end
#include <librpi/debug.h>           // For debug functions
#include <arinc653/partition.h>     // For RETURN_CODE_TYPE and
                                    // SET_PARTITION_MODE
#include <libpartition/scheduler.h> // For scheduler state

//==========================================================
// Lov-level init function that mainly inits memory.
// Can be called with only a stack set up, and sets up
// the entire execution context.
//==========================================================
void memory_init() {
  // Now, debug printing functions with puts (which
  // only requires a valid stack).
  // debug_puts("\tmemory_init: starting.\n") ;

  // Set up the stack_base (actually, simply copy the
  // one set by the L1 scheduler in the internal data
  // structure. It does not matter if there are
  // a few words left to successive function calls.
  {
    register uint32_t SP asm ("sp") ;
    l2_scheduler_state.mmap.stack_base = SP ;
  }
  
  // Zeroing the bss is not needed because it's done by
  // the L1 scheduler as part of L1_reset_partition.

  // Set up the heap just after the end of the
  // program segments.
  {
    uint32_t heap_base = (uint32_t)&_end  ;
    // Some alignment may be needed, because the
    // heap is aligned at 4-word barrier, not 1-word
    // cf. libc/malloc.c.
    if(((heap_base>>4)<<4)!=heap_base) {
      heap_base = ((heap_base>>4)+1)<<4 ;
    }
    // Set the value in the scheduler state.
    l2_scheduler_state.mmap.heap_base = heap_base ;
    l2_scheduler_state.mmap.heap_size = PARTITION_HEAP_SIZE ;
    // Actually init of the heap data structures.
    init_malloc((char*)l2_scheduler_state.mmap.heap_base,
		l2_scheduler_state.mmap.heap_size) ;
  }

  // Set up the point where stacks can be allocated for new
  // processes, after leaving place for the scheduler and
  // elaboration function stack.
  l2_scheduler_state.mmap.free_process_stack =
    l2_scheduler_state.mmap.stack_base
    - PARTITION_L2_STACK_SIZE ;

  // Low-level init completed.
  /*
  debug_puts("\tmemory_init: completed.\n") ;
  debug_printf("\tpartition memory organization:\n"
	       "\t   text_start:         %8x\n"
	       "\t   heap:               %8x - %8x\n"
	       "\t   stack base:         %8x\n"
	       "\t   process stack base: %8x\n",
	       &_text_start,
	       l2_scheduler_state.mmap.heap_base,
	       l2_scheduler_state.mmap.heap_base+
	       l2_scheduler_state.mmap.heap_size,
	       l2_scheduler_state.mmap.stack_base,
	       l2_scheduler_state.mmap.free_process_stack) ;
  */
}

//==========================================================
// Init function which sets everything to 0
//==========================================================
void L2_start_mode() {
  //
  //debug_puts("L2_start: enter COLD/WARM_START mode.\n") ;
  // This should be done only once after a reset (cannot go back
  // from NORMAL mode to COLD/WARM_START mode.
  if(l1_partition_interface.is_initialized) {
    debug_puts("L2_start: error: partition already initialized.\n") ;
  }
  // Mark partition as initialized.
  l1_partition_interface.is_initialized = 1 ;
  
  // Low-level init (memory, mainly).
  memory_init() ;
  /*
  {
    extern char* indent_line_prefix ;
    const char str[] = "[\t\t\t" ;
    strcpy(indent_line_prefix,str) ;
  }
  */
  // Only one initialization is needed for the process table,
  // given that most of them have been taken care of by
  // bzero.
  l2_scheduler_state.running_process = -1 ;
  
  debug_puts("L2_start: init completed, entering elaboration.\n") ;
  
  // Now, branch to the partition elaboration code (which uses the
  // ARINC 653/APEX API). This function should change state to
  // NORMAL mode or simply continue to execute non-preemptively as a
  // sequential thread.
  main_process() ;

  // If this function terminates, enter the IDLE mode and do
  // nothing more. I could raise an error, but this seems more
  // conservative.
  RETURN_CODE_TYPE rc ;
  SET_PARTITION_MODE(IDLE,&rc) ;
  // Unreachable code to shut up the compiler.
  for(;;) ;
}
