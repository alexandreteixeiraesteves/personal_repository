
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
#include <librpi/debug.h>              // For debug_printf, debug_puts
#include <libc/stdio.h>                // Debug: unsigned2ascii (which
                                       // works in very poor contexts).
#include <libc/stdlib.h>               // For exit(), halt(), ...
#include <kernel/boot-aux.h>           // For _context_save_struct
#include <kernel/scheduler.h>          // For L1_scheduler

//---------------------------------------------------------
// Several types of errors are treated identically,
// by restarting the erroneous partition (memory
// access errors and undefined instruction errors).
// 
// This handler routine will simply check whether the
// error comes from the current partition. If not, 
// it will completely hang up execution.
// If the error comes from the partition, then simply
// restart the partition.
//
// The action to take:
// - Determine if the instruction belongs to the current
//   partition. If yes, re-initialize the partition.
// - If not, hang up the system.
//---------------------------------------------------------
void handle_partition_error() {
  // First, check whether the scheduler is correctly
  // initialized
  if((!l1_scheduler_state.scheduler_initialized)||
     (l1_scheduler_state.number_of_partitions==0)) {
    fatal_error("handle_partition_error: scheduler not initialized.\n") ;
  }
  // Get the current partition information from the
  // scheduler configuration.
  struct L1SchedulerPartitionState* current_partition = 
    &(l1_scheduler_state.partition_state[l1_scheduler_state.active_partition]) ;
  // Check whether the error comes from the partition
  // memory zone.
  if((_context_save_struct.pc>=current_partition->mmap.memory_base)&&
     (_context_save_struct.pc<current_partition->mmap.stack_base)) {
    debug_puts("handle_partition_error: reset partition state\n") ;
    L1_handle_partition_error(l1_scheduler_state.active_partition) ;
    /*
      This approach does not currently ensure the absence of 
      corruption of the partition text and rodata. To ensure it, 
      I'd have to either:
      - Fully reload the ELF file and reconfigure the partition while 
        the other are working. To do this, I'd need:
        * The ability for the system to function in isolation from my
          partition
	* Some way of loading the partition on the time slots of the
          time slots of the partition (using some kernel code that is
          ran at that time.
      - Just reset the state of the partition to the initial state.
        For this to work, the code and rodata of the partition should be 
        read-only (can be done) but the data should be loaded in a 
        non-contiguous read-write zone. This requires a smarter ELF loader.
    */
  } else {
    // The error is coming from kernel or driver code.
    // Hang up.
    fatal_error("handle_partition_error: kernel code error. Exiting...\n") ;
  }
}

//---------------------------------------------------------
// Undefined instruction exception handler.
// The processor has received an undefined instruction
// to execute. This can happen when jumping to an 
// incorrect address (where the random octets do not make 
// for a correct opcode). In general, it can also be used
// to virtualize the hardware, but we don't have
// virtualization here.
//---------------------------------------------------------
void undef_handler(){
  debug_printf("\n****** UNDEF: Saved PC=0x%x Saved SP=0x%x\n",
	       _context_save_struct.pc,
	       _context_save_struct.sp) ;
  handle_partition_error() ;
}

//---------------------------------------------------------
// Prefetch abort handler, called for instruction memory
// faults. 
// The parameters (obtained by the assembly language
// vector) are:
// - ifsr - instruction fault status register
// - ifar - instruction fault address register
// - far  - fault address register
//---------------------------------------------------------
void prefetch_abort_handler(uint32_t ifsr,
			    uint32_t ifar,
			    uint32_t far) {
  debug_printf("\n****** Prefetch abort: IFSR:0x%x IFAR:0x%x FAR:0x%x PC:0x%x\n",
	       ifsr, /* Instruction fault status register (IFSR)  */
	       ifar, /* Instruction fault address register (IFAR) */
	       far,  /* Fault address register (FAR)              */
	       _context_save_struct.pc) ; /* Saved PC             */  
  handle_partition_error() ; 
}

//---------------------------------------------------------
// Data abort handler, called for data access faults. 
// The parameters (obtained by the assembly language
// vector) are:
// - dfsr - data fault status register
// - far  - fault address register
//---------------------------------------------------------
void data_abort_handler(uint32_t dfsr,
			uint32_t far) {
  debug_printf("\n****** Data abort: DFSR:0x%x FAR:0x%x PC:0x%x SP:0x%x\n",
	       dfsr,
	       far,
	       _context_save_struct.pc,
	       _context_save_struct.sp) ;
  handle_partition_error() ; 
}

//==================================================================
// Unhandled exceptions.

//---------------------------------------------------------
// Fast IRQ handler.
// Not used, currently, but for safety I put here an
// execution blocking behavior.
//---------------------------------------------------------
void fiq_handler() {
  fatal_error("\n$$$$$ FIQ: Unhandled... Exiting.\n") ;
}

//---------------------------------------------------------
// One interrupt vector has no exception associated with
// it. Just to be sure, I use for this vector a default
// handler that blocks execution.
//---------------------------------------------------------
void  unused_exception_vector() {
  fatal_error("\n$$$$$ Unused vector called. Exiting.\n") ;
}
