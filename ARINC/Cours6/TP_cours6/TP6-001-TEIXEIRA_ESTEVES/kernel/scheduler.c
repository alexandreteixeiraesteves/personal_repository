
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
#include <librpi/system_timer.h>  // For the system timer stuff
#include <librpi/mmu.h>           // For change_page_table
#include <librpi/debug.h>         // For debug_printf
#include <libc/string.h>          // For bzero
#include <libc/util.h>            // For increment_modulo
#include <libc/stdlib.h>          // For fatal_error

#include <kernel/boot-aux.h>      // For restore_full_context
#include <kernel/scheduler.h>

#define debug_printf(...) debug_printf(__VA_ARGS__)

//========================================================================
// L1 scheduler state, statically allocated in the data segment.
struct L1SchedulerState l1_scheduler_state ;

//========================================================================
// L1_scheduler routine
// This one is called for every scheduling event that gives
// control to the L2 scheduler. The only things that do not
// go through L1_scheduler are the simple requests that do not
// change the scheduling state (GET_TIME, calls resulting in
// NO_ACTION or errors, etc.).

//------------------------------------------------------------------------
// Time advance subroutine and associated actions (called by L1_scheduler
// only).

// These 3 functions are called upon specific scheduling events:
// tick, window change, partition change. Depending on what is put
// inside, one can parameter when certain actions are perfomed.
// For instance, when VL input is read (at partition
// switch, at window start, or at tick barrier).
// These functions are guaranteed to execute after the possible
// context changes related to the tick (partition switch) are
// performed. When several of them execute, the order is:
// tick before window before partition (cf. advance_time, last lines).
void partition_change_actions() {
}
void window_change_actions() {
}
void tick_actions() {
  //debug_printf("tick.\n") ;
}

// This function is called when I need to change the context
// during a tick timer handler. This function handles low-level
// aspects, such as the page table change or (in the future)
// the flush of the caches.
void partition_context_change(uint32_t new_partition) {
  // Debug trace
  /*
  debug_printf("partition_context_change: %2x->%2x. New perms: %x\n",
	       l1_scheduler_state.active_partition,
	       new_partition,
	       l1_scheduler_state.
	       partition_state[new_partition].dacr.bitvector) ;
  */
  // Switch the page table.
  change_page_table(l1_scheduler_state.page_table,
		    l1_scheduler_state.
		    partition_state[new_partition].dacr) ;
  // Change the active partition in the scheduler
  l1_scheduler_state.active_partition = new_partition ;
}

// This function is called when handling timer calls
// to advance the time and change L1 scheduler state
// accordingly. It only reads and changes the L1
// scheduler state.
void advance_time() {
  uint32_t partition_change = 0 ;
  uint32_t window_change = 0 ;
  // Advance the tick counter by 1 (modulo MTF) and update
  // (if needed) the active window and active partition.
  l1_scheduler_state.tick_counter =
    increment_modulo(l1_scheduler_state.tick_counter,
		     l1_scheduler_state.mtf) ;
  if(l1_scheduler_state.tick_counter == 0) {
    l1_scheduler_state.mtf_start_counter ++ ;
  }
  // If needed, change the window, which potentially changes the
  // active partition.
  if((l1_scheduler_state.tick_counter==0)||
     (l1_scheduler_state.tick_counter>=
      l1_scheduler_state.window_list[l1_scheduler_state.
				     active_window].end_date)) {
    window_change = 1 ;
    {
      // Change the window and perform a basic sanity check.
      l1_scheduler_state.active_window =
	increment_modulo(l1_scheduler_state.active_window,
			 l1_scheduler_state.number_of_windows) ;
      // Sanity check.
      if(l1_scheduler_state.tick_counter !=
	 l1_scheduler_state.window_list[l1_scheduler_state.
					active_window].start_date) {
	fatal_error("advance_time: incoherency in timing and "
		    "window accounting.\n") ;
      }
    }
    // If the window changes, then I usually (but not always) have to
    // change the partition, which includes changing the page table,
    // etc.).
    uint32_t new_partition = 
      l1_scheduler_state.window_list[l1_scheduler_state.
				     active_window].partition_id ;
    if(new_partition != l1_scheduler_state.active_partition) {
      partition_change = 1 ;
      partition_context_change(new_partition) ;
    }
  }
  // For each partition, increment the time since last tick.
  int i ;
  for(i=0;i<l1_scheduler_state.number_of_partitions;i++) {
    l1_scheduler_state.partition_state[i].
      system_ticks_since_last_partition_tick ++ ;
  }
  // Actions.
  tick_actions() ;
  window_change_actions() ;
  partition_change_actions() ;
}


//------------------------------------------------------------------------
// This function contains all the code needed to
// save the context of either the L2 scheduler or
// one process, and also log the L2 scheduler
// request.
// In the case of a process, saving is done on the
// cell pointed by eq.last, but without incrementing
// the eq.last.
// Note: ideally, the L1 scheduler should not know what
// the partition is doing, except for the partition mode
// information. However, the fact that it's the L1 kernel
// that must do all atomic operations means that for now
// it knows what is a process call and what is an L2 kernel
// call. The kernel context is saved in the L1 data
// structure (L2 sched. does not need it). The process
// context is saved in the interface.
//
// Whenever a process context is present, the L2 scheduler
// saves it and resets the process_context_saved flag.
// This approach, where the process contexts are not
// timestamped, is correct because I only care about the
// official status of a process when a timer arrives. The
// intermediate data is not important.
void save_partition_context() {
  struct L1SchedulerPartitionState* partition_state =
    &l1_scheduler_state.
    partition_state[l1_scheduler_state.active_partition] ;
  if(partition_state->in_a_process) {
    // Save the context in the process-specific data
    // structure.
    struct L1PartitionInterface* interface =
      partition_state->mmap.interface ;
    interface->process_context_saved = 1 ;
    interface->process_context = _context_save_struct ;
  } else {
    // Save the context in the scheduler-specific data structure
    partition_state->l2_scheduler_context = _context_save_struct ;
  }
}

//------------------------------------------------------------------------
// Create one (or more, in the future) L2 event(s) in the
// event queue of the partition. 
// This context must **not** be over-written.
void create_L2_event(enum L2_event_type l2_event_type,
		     void* l2_event_data_ptr) {
  //debug_printf("create_L2_event: entered with event type %d.\n",
  //             l2_event_type) ;
  if(l2_event_type != NO_EVENT) {
    struct L1SchedulerPartitionState* partition_state =
      &l1_scheduler_state.
      partition_state[l1_scheduler_state.active_partition] ;
    struct L1PartitionInterface* interface =
      partition_state->mmap.interface ;
    struct L2_event* e = &interface->eq.events[interface->eq.last] ;
    {
      e->type = l2_event_type ;
      switch(l2_event_type) {
      case KERNEL_TICK_EVENT:
	// Timestamp the request, and then reset the time.
	e->d.kernel_tick.system_ticks_since_last_partition_tick =
	  partition_state->system_ticks_since_last_partition_tick ;
	partition_state->system_ticks_since_last_partition_tick = 0 ;
	e->d.kernel_tick.is_partition_periodic_start =
	  (l1_scheduler_state.tick_counter ==
	   l1_scheduler_state.window_list[l1_scheduler_state.active_window].
	   start_date) &&
	  l1_scheduler_state.window_list[l1_scheduler_state.active_window].
	  partition_period_start ;
	break ;
      case PARTITION_L2_EVENT:
	// All information is in the saved context, but for simple
	// access I put it here.
	e->d.partition_l2.req_data_ptr = l2_event_data_ptr ;
	break ;
      default:
	// Error
	fatal_error("save_partition_context: Unknown request type.") ;
	break ;
      }
    }
    // Increment the queue end pointer.
    interface->eq.last = increment_modulo(interface->eq.last,
					  EVENT_QUEUE_SIZE) ;
  }
  //debug_printf("create_L2_event: exited.\n") ;
}

//------------------------------------------------------------------------
// This function gives control to the L2 scheduler and
// ensures that the in_a_process flag is correctly set.
void call_L2_scheduler() {
  // Give control to the scheduler context saved in the partition
  // data structure. This encompasses both the start case and the
  // resume case.
  struct L1SchedulerPartitionState* curr_partition_state =
    &l1_scheduler_state.
    partition_state[l1_scheduler_state.active_partition] ;
  
  // Before giving control to that context, set the in_a_process
  // flag to false (because we give control to the scheduler).
  curr_partition_state->in_a_process = 0 ;
  restore_full_context(&curr_partition_state->l2_scheduler_context) ;
}

//------------------------------------------------------------------------
// This routine is the entry point of the scheduler. 
// Coding it has several constraints:
// - Its stack is already set to a fixed value, which
//   is the same at each call, meaning that all context
//   should be saved in static variables.
// - It does not return, and interrupts are not enabled
//   during its execution (meaning it should be kept 
//   short).
void L1_scheduler(enum L2_event_type l2_event_type,
		  void* l2_event_data_ptr) {
  // debug_puts("L1_scheduler: entered.\n");
  struct L1PartitionInterface* interface =
    l1_scheduler_state.
    partition_state[l1_scheduler_state.active_partition].
    mmap.interface ;
  // If the partition was already initialized, save the
  // context to the appropriate place (process context or
  // L2 scheduler context). If the partition was not yet
  // initialized, nothing can be saved and the correct
  // entry point context has alredy been set during init
  // by L1_reset_partition.
  if(interface->is_initialized) {
    // Save context and log the L2 scheduler request.
    save_partition_context() ;
  }

  // First thing to do: advance logical time.
  // Only upon timer requests. This implies that the
  // partition that was interrupted may not be the one
  // that gets restarted.
  if(l2_event_type == KERNEL_TICK_EVENT) {
    advance_time() ;
    // Advancing time can also change the active partition
    // and/or window. I therefore need to update the interface
    // pointer.
    interface =
      l1_scheduler_state.
      partition_state[l1_scheduler_state.active_partition].
      mmap.interface ;
  }

  // Second thing: If the partition is in NORMAL mode,
  // I need to inform the L2 scheduler of the scheduling
  // decisions made here.
  if(interface->partition_status.OPERATING_MODE == NORMAL) {
    create_L2_event(l2_event_type,
		    l2_event_data_ptr) ;
  }

  // Give control to the L2 scheduler of the current
  // partition, loading the saved context.
  call_L2_scheduler() ;
}

//==========================================================================
// L1_set_partition_mode and aux functions.
// Among aux functions, I also make public L1_reset_partition, which is
// used in the initialization code.
//
// L1_set_partition_mode is only called from the SVC handler.
//==========================================================================

// This function creates a return context corresponding to the
// restart of the L2 scheduler function. This is needed to
// allow the correct switching of modes.
void reset_entry_point(PARTITION_ID_TYPE pid,
		       struct FullRegisterSet* rs) {
  // Put all values to 0
  bzero((char*)rs,
	sizeof(struct FullRegisterSet)) ;
  // Set up just 3 registers: PC, SP, and CPSR
  rs->pc = l1_scheduler_state.partition_state[pid].mmap.entry_point ;
  rs->sp = l1_scheduler_state.partition_state[pid].mmap.stack_base ;
  CPSR_type cpsr ;
  {
    cpsr.bv = 0 ;
    cpsr.decoder.F = 1 ; // FIQs not accepted
    if(pid == 0) {
      // System partition
      cpsr.decoder.M = CPSR_MODE_SYSTEM ;
    } else {
      // User partition
      cpsr.decoder.M = CPSR_MODE_USER ;
    }
  }
  rs->cpsr = cpsr ;
}

// If a partition does not change the values of initialized variables
// (or does not have such variables) then calling this function resets
// the partition, and it can be restarted from scratch.
// The only constraint is that the following variables have correct
// values:
// - mtf, tick_length (from the top-level L1SchedulerState
// - all the fields of mmap
// - init_partition_status
// - dacr
void L1_reset_partition(PARTITION_ID_TYPE pid) {
  // First, reset the interface pointed by the mmap.interface.
  {
    struct L1PartitionInterface* interface =
      l1_scheduler_state.partition_state[pid].mmap.interface ;
    // Put bss to zero (this also bzeroes the interface in the current
    // configuration, but for future compatibility I keep a second
    // bzero afterwards).
    bzero((char*)l1_scheduler_state.partition_state[pid].mmap.bss_base,
	  l1_scheduler_state.partition_state[pid].mmap.bss_size) ;
    // Put all interface to 0
    bzero((char*)interface,sizeof(struct L1PartitionInterface)) ;
    // Copy the init status and RT characteristics from the master copy.
    interface->tick_length = l1_scheduler_state.tick_length ;
    interface->partition_status =
      l1_scheduler_state.partition_state[pid].init_partition_status ;
    // Note that the following fields are initialized with correct
    // values (0) by bzero: eq, is_initialized, process_context_saved,
    // process_context.
  }
  // Next, reset the L1 partition state, which resets the
  // saved scheduler context. 
  reset_entry_point(pid,
		    &l1_scheduler_state.partition_state[pid].
		    l2_scheduler_context) ;
  // Remaining fields:
  l1_scheduler_state.partition_state[pid].
    system_ticks_since_last_partition_tick = 0 ; // Restart time from 0
  l1_scheduler_state.partition_state[pid].
    in_a_process = 0 ; // Not in a process.
}

// This function performs partition state changes.
// It is called upon partition change requests, external or
// internal.
uint32_t L1_set_partition_mode(OPERATING_MODE_TYPE new_mode) {
  if(l1_scheduler_state.active_partition == 0) {
    // System partition. Currently, in this partition scheduling is
    // not preemptive, so it should never change from the WARM_START
    // state after logging an error.
    debug_puts("L1_set_partition_mode: error: system partition call.\n") ;
    return NO_ACTION ;
  } else {
    //    debug_puts("L1_set_partition_mode: A\n") ;
    // Extract some data
    struct L1PartitionInterface* interface =
      l1_scheduler_state.
      partition_state[l1_scheduler_state.active_partition].mmap.
      interface ;
    OPERATING_MODE_TYPE curr_mode =
      interface->partition_status.OPERATING_MODE ;
    // Check if the change is legal. If not, return the error code
    // prescribed by the ARINC 653 standard.
    if((new_mode == NORMAL)&&(curr_mode == NORMAL)) return NO_ACTION ;
    if((new_mode == WARM_START)&&(curr_mode == COLD_START))
      return INVALID_MODE ;
    if((new_mode != IDLE) &&
       (new_mode != COLD_START) &&
       (new_mode != WARM_START) &&
       (new_mode != NORMAL)) {
      return INVALID_PARAM ;
    }
    //    debug_puts("L1_set_partition_mode: B\n") ;
    // Now, the real actions in case the change is legal.
    switch(new_mode) {
    case NORMAL:
      // In addition to what is done in the IDLE case, for the
      // NORMAL mode I need to reset the tick counter of the
      // partition.
      l1_scheduler_state.
	partition_state[l1_scheduler_state.active_partition].
	system_ticks_since_last_partition_tick = 0 ;
    case IDLE:
      //      debug_puts("L1_set_partition_mode: C\n") ;
      // If the new mode is NORMAL, then I need to do nothing
      // (the system_ticks_to_next_partition_tick is already 0
      // due to the initial partition reset).
      // First, change the status in the interface.
      interface->partition_status.OPERATING_MODE = new_mode ;
      // Second, switch the context to force restart of the L2
      // scheduler function. L1_scheduler will do the rest
      // of the job. Attention, the context alteration must be
      // done on _context_save_struct, which the L1 scheduler
      // will save next...
      reset_entry_point(l1_scheduler_state.active_partition,
			&_context_save_struct) ;
      break ;
    case COLD_START:
    case WARM_START:
      //      debug_puts("L1_set_partition_mode: D\n") ;
      // Fully reset the partition (which always enters
      // WARM_START mode).
      L1_reset_partition(l1_scheduler_state.active_partition) ;
      break ;
    default:
      //      debug_puts("L1_set_partition_mode: E\n") ;
      // This case was tested above. If it happens, it's a big bug.
      // I stop execution altogether.
      fatal_error("L1_set_partition_mode: fatal internal "
			 "kernel error.\n") ;
    }
    //    debug_puts("L1_set_partition_mode: F\n") ;
    // And now, give control back to the L2 scheduler, by calling
    // the L1_scheduler function. This does not return.
    L1_scheduler(NO_EVENT,NULL) ;
    // Unreachable code, to shut up the compiler
    return NO_ERROR ;
  }
}

//==========================================================================
// L1_restore_process_context
// Restore a process context at the request of the L2 scheduler, and
// at the same time save the L2 scheduler context to allow its resumption
// later. This is why the function takes 2 arguments.
//
// Called from the SVC handler only.
//==========================================================================

// This is the mechanism that calls processes in the partition.
// The caller context is that of the scheduler. It has to be
// saved. The target context is the process context to be
// restored.
void L1_restore_process_context(struct FullRegisterSet* l2_scheduler_context,
				struct FullRegisterSet* process_context) {
  // Save scheduler context.
  l1_scheduler_state.
    partition_state[l1_scheduler_state.active_partition].
    l2_scheduler_context = *l2_scheduler_context ;
  // Make sure the CPSR of the process context is not unsafe.
  process_context->cpsr.bv = 0 ;
  process_context->cpsr.decoder.M = CPSR_MODE_USER ;
  process_context->cpsr.decoder.F = 1 ;
  process_context->cpsr.decoder.I = 0 ;
  process_context->cpsr.decoder.T = 0 ;  
  process_context->cpsr.decoder.E = 0 ;  
  process_context->cpsr.decoder.A = 0 ;  
  // Set up the in_a_process flag
  l1_scheduler_state.
    partition_state[l1_scheduler_state.active_partition].
    in_a_process = 1 ;
  // Restore context
  restore_full_context(process_context) ;
}

//==========================================================================
// L1_handle_partition_error
// Reset the partition and then wait the next tick to restart.
//
// Called from the error handler routine handler_partition_error, itself
// called by several interrupt handlers (undef, prefetch_abort,
// data_abort).
//==========================================================================
void L1_handle_partition_error(PARTITION_ID_TYPE partition_id) {
  // Reset partition.
  L1_reset_partition(partition_id) ;
  // Restart it.
  L1_scheduler(NO_EVENT,NULL) ;
}

//==========================================================================
// L1_get_time.
// This function returns the time spent by the system since the
// start of the first MTF. The duration is ceiled to the tick_length
// size (time spent in the current tick is not taken into account).
// Time is in microseconds, like tick_length.
//
// Called from the SVC handler upon GET_TIME requests.
//==========================================================================
SYSTEM_TIME_TYPE L1_get_time() {
  SYSTEM_TIME_TYPE res = l1_scheduler_state.mtf_start_counter ;
  res *= (l1_scheduler_state.mtf-1) ;
  res += l1_scheduler_state.tick_counter ;
  res *= l1_scheduler_state.tick_length ;
  return res ;
}
