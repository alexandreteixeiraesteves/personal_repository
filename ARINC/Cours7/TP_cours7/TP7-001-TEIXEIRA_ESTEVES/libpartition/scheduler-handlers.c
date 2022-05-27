
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
#include <libc/string.h>                 // For bzero
#include <libc/stdlib.h>                 // For fatal_error
#include <librpi/debug.h>                // For debug_printf
#include <arinc653/types.h>              // For INFINITE_TIME_VALUE
#include <arinc653/process.h>            // For PROCESS_ID_TYPE
#include <libpartition/scheduler.h>      // For the scheduler data structs
#include <libpartition/svc-l2-request.h> // For svc_l2_request_stop.


void handler_stop_process(PROCESS_ID_TYPE target_pid,
			  PROCESS_ID_TYPE caller_pid) {
  // Put the target process in DORMANT mode.
  {
    struct L2_process_record * target_process =
      &l2_scheduler_state.process_table[target_pid] ;
    // Put this process in DORMANT mode.
    target_process->process_status.PROCESS_STATE = DORMANT ;
    // Remove the process from the queue.
    remove_from_process_queue(target_pid) ;
  }
  // If the caller is a process different from the target,
  // then I have to change its context to set r0 to
  // NO_ERROR (the return code). Thus, the call to STOP
  // completes smoothly.
  if((caller_pid >= 0)&&
     (caller_pid != target_pid)) {
    l2_scheduler_state.process_table[caller_pid].context.r[0] = NO_ERROR ;
  }
}

// This is the standard exit function of all tasks,
// which ensures that terminating the user-provided function
// results in the task stopping (becoming DORMANT) with an
// explicit scheduler event.
void handler_process_exit() {
  STOP_SELF() ;
  for(;;) ;
}

// This is the function that actually does what
// DELAYED_START has to do. The DELAYED_START function
// is only handling the consistency checks.
RETURN_CODE_TYPE handler_delayed_start(PROCESS_ID_TYPE target_pid,
				       PROCESS_ID_TYPE caller_pid,
				       SYSTEM_TIME_TYPE delay) {
  RETURN_CODE_TYPE rc ;
  // Shortcut
  struct L2_process_record * target_process =
    &l2_scheduler_state.process_table[target_pid] ;
  // Set priority
  target_process->process_status.CURRENT_PRIORITY =
    target_process->process_status.ATTRIBUTES.BASE_PRIORITY ;
  
  // Init scheduling queue info.
  target_process->next_in_queue = -1 ;

  // The standard says here "reset context and stack".
  // I simply do this by reseting the register context.
  {
    struct FullRegisterSet* context =
      &l2_scheduler_state.process_table[target_pid].context ;
    // Reset all registers to 0
    bzero(context,sizeof(struct FullRegisterSet)) ;
    // Set the stack and entry point.
    context->sp =
      target_process->process_stack_base ;
    context->pc =
      (uint32_t)target_process->process_status.ATTRIBUTES.ENTRY_POINT ;
    context->lr =
      // This is the address of a function that will spend time
      // doing nothing. It should not be reached by any task, because
      // a task is not supposed to terminate, but call STOP_SELF and
      // become dormant.
      (uint32_t)handler_process_exit ;
    // The CPSR is anyways corrected by the L1 kernel for the
    // safety-critical parts (nothing to do).
  }
  
  //
  if(target_process->process_status.ATTRIBUTES.PERIOD > 0) {
    // Periodic process.
    // This case is simpler, because I only wait for one
    // cause -- the first partition periodic start -- regardless
    // of the mode I'm in (NORMAL or START).
    target_process->process_status.PROCESS_STATE = WAITING ;
    target_process->next_release_point = delay ;
    target_process->waiting_cause = WaitPartitionPeriodStart ;
    rc = NO_ERROR ;
  } else {
    // Aperiodic process. I have two main cases here:
    // - called in START mode
    // - called in NORMAL mode
    // In the first case, the delay applies from the start
    // of the NORMAL mode. In the second, from the current
    // tick.
    switch(l1_partition_interface.partition_status.OPERATING_MODE) {
    case COLD_START:
    case WARM_START:
      target_process->process_status.PROCESS_STATE = WAITING ;
      target_process->next_release_point = delay ;
      target_process->waiting_cause = WaitPartitionPeriodStart ;
      break ;
    case NORMAL:
      // Not implemented. Leave in DORMANT state.
      rc = NOT_AVAILABLE ;
      break ;
    case IDLE:
    default:
      // Just in case there are weird errors.
      fatal_error("handler_delayed_start:Called in bad mode.\n") ;
    }
  }
  
  // Set return code.
  // If the caller is a process, then I have to change its context
  // to set r0 to the correct return code. Thus, the call to DELAYED_START
  // completes smoothly.
  if(caller_pid >= 0) {
    l2_scheduler_state.process_table[caller_pid].context.r[0] = rc ;
  }
  return rc ;
}

//
void handler_enter_normal_mode() {
  int i ;
  for(i=0;i<MAX_PROCESS_NUMBER;i++) {
    struct L2_process_record* current_process =
      &l2_scheduler_state.process_table[i] ;
    if(current_process->record_used) {
      if((current_process->process_status.PROCESS_STATE == WAITING) &&
	 (current_process->waiting_cause == WaitNormalMode)) {
	// This process is now ready. No need to reset the waiting cause,
	// it will be when I reenter the waiting state.
	current_process->process_status.PROCESS_STATE = READY ;
	// Insert this process in the process queue.
	insert_into_process_queue(i) ;
      }
    }
  }
}

//
void handler_tick_event(uint32_t system_ticks_since_last_partition_tick,
			uint32_t is_partition_periodic_start) {
  uint32_t deadline_miss_bitvector = 0 ;
  // Update the scheduler control structures to account for
  // the passage of time. For processes that are WAITING,
  // there is a case for periodic processes if
  // partition_period_start is true.
  PROCESS_ID_TYPE pid ;
  for(pid=0;pid<MAX_PROCESS_NUMBER;pid++) {
    if(l2_scheduler_state.process_table[pid].record_used) {
      struct L2_process_record * current_process =
	&l2_scheduler_state.process_table[pid] ;
      PROCESS_STATE_TYPE current_state =
	current_process->process_status.PROCESS_STATE ;
      if(current_state != DORMANT) {
	if((current_state == WAITING) &&
	   (current_process->waiting_cause == WaitPartitionPeriodStart)) {
	  if(is_partition_periodic_start) {	   
	    // This case corresponds to periodic processes that wait for
	    // partition period starts (PPS). When they reach their first
	    // PPS, their configuration is completed and they enter normal
	    // time-driven scheduling.
	    //
	    // First, the final configuration elements:
	    // Change the waiting cause to waiting on the periodic
	    // timer.
	    current_process->waiting_cause = WaitPeriodicTimer ;
	    // The next release point is the
	    // current_process->next_release_point (the initial value
	    // is the delay, which applies from the current time point).
	    //
	    // Instead, I do need to update the deadline, which I
	    // set to the delay plus the time capacity of the
	    // process.
	    if(current_process->process_status.ATTRIBUTES.TIME_CAPACITY
	       == INFINITE_TIME_VALUE) {
	      // Set a value that does not trigger deadlines
	      current_process->process_status.DEADLINE_TIME = 1 ;
	    } else {
	      // If there is a deadline to set.
	      current_process->process_status.DEADLINE_TIME =
		current_process->next_release_point +
		current_process->process_status.ATTRIBUTES.TIME_CAPACITY ;
	    }
	  }
	} else {
	  if(current_process->process_status.ATTRIBUTES.TIME_CAPACITY
	     != INFINITE_TIME_VALUE) {
	    // If there is a deadline to set.
	    // Normal timing accounting for a fully configured process
	    // First, downcount the deadlines.
	    current_process->process_status.DEADLINE_TIME
	      -= system_ticks_since_last_partition_tick ;
	    // For periodic processes, I need to update the next
	    // release point.
	    if(current_process->process_status.ATTRIBUTES.PERIOD != 0) {
	      current_process->next_release_point
		-= system_ticks_since_last_partition_tick ;
	    }
	  }
	  // If a timeout is ongoing, downcount it.
	  if((current_state == WAITING) &&
	     (current_process->waiting_cause == WaitTimer)) {
	    current_process->timer_timeout
	      -= system_ticks_since_last_partition_tick ;
	  }
	}
	// Now, check for timeout events for processes that are in
	// RUNNING or READY state, or in WAITING state where waiting is
	// "active" (the task is running, not waiting to start). Recall
	// that a process can be waiting to start execution.
	if(current_process->process_status.ATTRIBUTES.TIME_CAPACITY
	   != INFINITE_TIME_VALUE) {
	  if(!((current_state==WAITING)&&
	       ((current_process->waiting_cause == WaitPartitionPeriodStart)||
		(current_process->waiting_cause == WaitPeriodicTimer)))) {
	    // Deadline check is done 
	    if(current_process->process_status.DEADLINE_TIME <= 0) {
	      // Deadline miss, that needs to be handled (at the same time for
	      // all processes).
	      deadline_miss_bitvector |= 1 << pid ;
	    }
	  }
	}
	if(current_state == WAITING) {
	  switch(current_process->waiting_cause) {
	  case WaitTimer:
	    // When a process arrives here, it is suspended on
	    // a TIMED_WAIT
	    if(current_process->timer_timeout <= 0) {
	      current_process->process_status.PROCESS_STATE = READY ;
	      insert_into_process_queue(pid) ;
	    }
	    break ;
	  case WaitPeriodicTimer:
	    // When a process arrives here, it is self-suspended
	    // on a periodic wait. It has to be restarted.
	    if(current_process->next_release_point <= 0) {
	      current_process->process_status.PROCESS_STATE = READY ;
	      current_process->process_status.DEADLINE_TIME =
		current_process->next_release_point +
		current_process->process_status.ATTRIBUTES.TIME_CAPACITY ;
	      current_process->next_release_point +=
		current_process->process_status.ATTRIBUTES.PERIOD ;
	      insert_into_process_queue(pid) ;
	    }
	    break ;
	  default:
	    break ;
	  }
	}
      }
    }
  }
  // Handle deadline misses, after everything else time-dependent
  // has been done.
  if(deadline_miss_bitvector) {
    for(pid=0;pid<MAX_PROCESS_NUMBER;pid++) {
      if(deadline_miss_bitvector & (1<<pid)) {
	// Deadline miss on process pid.
	// Current action: stop the process.
	// I cannot directly call STOP, because it may lead to problems
	// afterwards. Instead, I have to make the low-level call myself.
	debug_printf("handler_tick_event: deadline.\n") ;
	svc_l2_request_stop(pid,-1) ;
      }
    }
  }
}



RETURN_CODE_TYPE handler_resume(PROCESS_ID_TYPE target_pid,
				PROCESS_ID_TYPE caller_pid) {
  RETURN_CODE_TYPE rc ;
  if(target_pid == caller_pid) {
    // I did not check here that target_pid is a valid pid.
    rc = INVALID_PARAM ;
  } else {
    struct L2_process_record * target_process =
      &l2_scheduler_state.process_table[target_pid] ;
    if(target_process->process_status.PROCESS_STATE == DORMANT) {
      rc = INVALID_MODE ;
    } else {
      if(target_process->process_status.ATTRIBUTES.PERIOD > 0) {
	rc = INVALID_MODE ;
      }
      if(target_process->waiting_cause!=WaitSuspended) {
	rc = NO_ACTION ;
      } else {
	// TODO: add the timer stuff, currently not here.
	target_process->process_status.PROCESS_STATE = READY ;
	// Insert this process in the process queue.
	insert_into_process_queue(target_pid) ;
	rc = NO_ERROR ;
      }
    }
  }
  // Set return code.
  // If the caller is a process, then I have to change its context
  // to set r0 to the correct return code. Thus, the call to RESUME
  // completes smoothly.
  if(caller_pid >= 0) {
    l2_scheduler_state.process_table[caller_pid].context.r[0] = rc ;
  }
  return rc ;
}


// Handling of periodic_wait requests.
void handler_partition_l2_periodic_wait() {
  struct L2_process_record * running_process =
    &l2_scheduler_state.process_table[l2_scheduler_state.running_process] ;
  // Sanity check
  if((running_process->process_status.ATTRIBUTES.PERIOD <= 0)
     ||(running_process->process_status.PROCESS_STATE != RUNNING)){
    debug_printf("handler_partition_l2_periodic_wait: called on "
		 "aperiodic or non-running process.\n") ;
  } else {
    // Put this process in WAITING mode and change the
    // waiting cause.
    running_process->process_status.PROCESS_STATE = WAITING ;
    running_process->waiting_cause = WaitPeriodicTimer ;
    // No need to update the deadline or next_release_point.
    // These are done in handler_tick_event, each time the
    // process is started.
    // Return NO_ERROR
    running_process->context.r[0] = NO_ERROR ;
    // Remove the process from the queue.
    remove_from_process_queue(l2_scheduler_state.running_process) ;
  }
}

void handler_timed_wait(SYSTEM_TIME_TYPE delay) {
  //debug_printf("handler_timed_wait called with duration %d.\n",delay) ;
  struct L2_process_record * running_process =
    &l2_scheduler_state.process_table[l2_scheduler_state.running_process] ;
  // Sanity check
  if(running_process->process_status.PROCESS_STATE != RUNNING){
    debug_printf("handler_partition_l2_timed_wait: called on "
		 "non-running process.\n") ;
  } else {
    running_process->process_status.PROCESS_STATE = WAITING ;
    running_process->waiting_cause = WaitTimer ;
    running_process->timer_timeout = delay ;
    running_process->context.r[0] = NO_ERROR ;
    remove_from_process_queue(l2_scheduler_state.running_process) ;
  }
  //  debug_printf("handler_timed_wait finished.\n") ;  
}

RETURN_CODE_TYPE handler_suspend(PROCESS_ID_TYPE target_pid,
				 PROCESS_ID_TYPE caller_pid,
				 SYSTEM_TIME_TYPE delay) {
  //debug_printf("handler_suspend -- entering.\n") ;
  RETURN_CODE_TYPE rc ;
  if(delay != INFINITE_TIME_VALUE) {
    // UNSUPPORTED: delays are currently unsupported
    rc = INVALID_PARAM ;
  } else {
    struct L2_process_record * target_process =
      &l2_scheduler_state.process_table[target_pid] ;
    if(target_process->process_status.ATTRIBUTES.PERIOD > 0) {
      rc = INVALID_MODE ;
    } else {
      if(target_process->process_status.PROCESS_STATE == DORMANT) {
	rc = INVALID_MODE ;
      } else {
	switch(target_process->process_status.PROCESS_STATE){
	case WAITING:
	  switch(target_process->waiting_cause) {
	  case WaitSuspended :
	    rc = NO_ACTION ;
	    break ;
	  default:
	    target_process->waiting_cause = WaitSuspended ;
	    rc = NO_ERROR ;
	    break ;
	  }
	  break ;
	case RUNNING:
	case READY:
	  target_process->process_status.PROCESS_STATE = WAITING ;
	  target_process->waiting_cause = WaitSuspended ;
	  remove_from_process_queue(target_pid) ;
	  rc = NO_ERROR ;
	  break ;
	default:
	  fatal_error("handler_suspend: bad process state.") ;
	}
      }
    }
  }
  // Set return code.
  // If the caller is a process, then I have to change its context
  // to set r0 to the correct return code. Thus, the call to RESUME
  // completes smoothly.
  if(caller_pid >= 0) {
    l2_scheduler_state.process_table[caller_pid].context.r[0] = rc ;
  }
  return rc ;
}
