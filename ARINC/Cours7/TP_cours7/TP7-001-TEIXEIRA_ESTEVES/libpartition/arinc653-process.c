
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
#include <libc/stdlib.h>                 // For fatal_error
#include <libpartition/scheduler.h>      // For data structures.
#include <libpartition/svc-l2-request.h> // For SVC calls
#include <librpi/debug.h>                // For debug_printf
#include <arinc653/process.h>


void CREATE_PROCESS (/*in */ PROCESS_ATTRIBUTE_TYPE *ATTRIBUTES,
		     /*out*/ PROCESS_ID_TYPE *PROCESS_ID,
		     /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  if((l1_partition_interface.partition_status.OPERATING_MODE
      != WARM_START) &&
     (l1_partition_interface.partition_status.OPERATING_MODE
      != COLD_START)) {
    // Processes can only be started in COLD_/WARM_START mode.
    *RETURN_CODE = INVALID_MODE ;
    return ;
  }
  int32_t pid ;
  for(pid=0;
      (pid<MAX_PROCESS_NUMBER)&&
	l2_scheduler_state.process_table[pid].record_used;
      pid++) ;
  if(pid == MAX_PROCESS_NUMBER) {
    // There is no more place for this process.
    debug_printf("No place left.\n") ;
    *RETURN_CODE = INVALID_CONFIG ;
    return ;
  }
  // Found a record that is yet unused. Take it and fill it.
  // Start with process_status.
  {
    PROCESS_STATUS_TYPE tmp =
      {
	.ATTRIBUTES = *ATTRIBUTES,
	.DEADLINE_TIME = INFINITE_TIME_VALUE,
	.CURRENT_PRIORITY = ATTRIBUTES->BASE_PRIORITY,
	.PROCESS_STATE = DORMANT,
      } ;
    // Some normalization is needed on the ATTRIBUTES, because
    // all internal accounting is done in ticks.
    {
      if(tmp.ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE) {
	/* Periodic process */	
	if(tmp.ATTRIBUTES.PERIOD <= 0) {
	  // Period must be positive (or 0).
	  debug_printf("Non-positive period.\n") ;
	  *RETURN_CODE = INVALID_PARAM ;
	  return ;
	}
	if(tmp.ATTRIBUTES.PERIOD % l1_partition_interface.tick_length) {
	  // All periods must be multiple of the tick.
	  debug_printf("Period not multiple of tick.\n") ;
	  *RETURN_CODE = INVALID_CONFIG ;
	  return ;
	}
	// All internal bookkeeping is done in ticks, not in
	// microseconds, so all values are converted.
	tmp.ATTRIBUTES.PERIOD /= l1_partition_interface.tick_length ;
      }
      if(tmp.ATTRIBUTES.TIME_CAPACITY == INFINITE_TIME_VALUE) {
	if(tmp.ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE) {
	  // Time capacity cannot be infinite for periodic process.
	  debug_printf("Infinite time capacity for periodic process.\n") ;
	  *RETURN_CODE = INVALID_PARAM ;
	  return ;
	}
      } else {
	if(tmp.ATTRIBUTES.TIME_CAPACITY <= 0) {
	  // Time capacity must be strictly positive.
	  debug_printf("Time capacity is negative.\n") ;
	  *RETURN_CODE = INVALID_PARAM ;
	  return ;
	}
	if(tmp.ATTRIBUTES.TIME_CAPACITY % l1_partition_interface.tick_length) {
	  // All capacities must be multiple of the tick.
	  debug_printf("Time capacity is not multiple of tick.\n") ;
	  *RETURN_CODE = INVALID_CONFIG ;
	  return ;
	}
	tmp.ATTRIBUTES.TIME_CAPACITY /= l1_partition_interface.tick_length ;
	if((tmp.ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE)
	   &&(tmp.ATTRIBUTES.TIME_CAPACITY > tmp.ATTRIBUTES.PERIOD)){
	  // Time capacity cannot be greater than period.
	  debug_printf("Time capacity greater than period.\n") ;
	  *RETURN_CODE = INVALID_PARAM ;
	  return ;
	}
      }
    }
    // And now, transfer it into the process record.
    l2_scheduler_state.process_table[pid].process_status = tmp ;
  }
  
  // Stack initialization
  {
    // Sanity check: make sure the stack is larger than a minimal value.
    if(l2_scheduler_state.process_table[pid].process_status.
       ATTRIBUTES.STACK_SIZE
       < MIN_PROCESS_STACK_SIZE) {
      debug_printf("Stack is too small.\n") ;
      *RETURN_CODE = INVALID_PARAM ;
      return ;
    }
    // Make sure free_process_stack is word-aligned. This
    // under-approximation is safe for the stack because it grows
    // down.
    l2_scheduler_state.mmap.free_process_stack =
      (l2_scheduler_state.mmap.free_process_stack>>2)<<2 ;
    // Compute its new value.
    uint32_t new_free_process_stack =
      ((l2_scheduler_state.mmap.free_process_stack -
	l2_scheduler_state.process_table[pid].
	process_status.ATTRIBUTES.STACK_SIZE)>>2)<<2 ;
    if(new_free_process_stack <=
       l2_scheduler_state.mmap.heap_base + l2_scheduler_state.mmap.heap_size) {
      // No place for the process stack
      debug_printf("No place for process stack.\n") ;
      *RETURN_CODE = INVALID_PARAM ;
      return ;
    }
    l2_scheduler_state.process_table[pid].process_stack_base = 
      l2_scheduler_state.mmap.free_process_stack ;
    l2_scheduler_state.mmap.free_process_stack =
      new_free_process_stack ;
  }  
  
  // Set up the record to show it's dormant (not in the scheduling
  // queue).
  l2_scheduler_state.process_table[pid].next_in_queue = -1 ;
  
  // No need to init timer_timeout, next_release_point,
  // context, waiting_cause.
  *PROCESS_ID = pid ;
  // Set up the return code
  *RETURN_CODE = NO_ERROR ;
  // Only after everything works I fully reserve the
  // process record and return.
  l2_scheduler_state.process_table[pid].record_used = 1 ;
  return ;
}

void DELAYED_START (/*in */ PROCESS_ID_TYPE PROCESS_ID,
		    /*in */ SYSTEM_TIME_TYPE DELAY_TIME,
		    /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  // Sanity
  if((PROCESS_ID < 0)||(PROCESS_ID >= MAX_PROCESS_NUMBER)) {
    *RETURN_CODE = INVALID_PARAM ;
    return ;
  }
  // Sanity
  if(!l2_scheduler_state.process_table[PROCESS_ID].record_used) {
    *RETURN_CODE = INVALID_PARAM ;
    return ;
  }
  // Sanity
  if(l2_scheduler_state.process_table[PROCESS_ID].process_status.PROCESS_STATE
     != DORMANT) {
    *RETURN_CODE = NO_ACTION ;
    return ;
  }
  // Sanity
  if((DELAY_TIME<0)||
     (DELAY_TIME%l1_partition_interface.tick_length)) {
    // DELAY_TIME must be positive (or 0) and a multiple of tick length.
    *RETURN_CODE = INVALID_PARAM ;
    return ;
  }
  // Convert delay in ticks.
  DELAY_TIME = DELAY_TIME / l1_partition_interface.tick_length ;
  // Sanity
  if(l2_scheduler_state.process_table[PROCESS_ID].process_status.
     ATTRIBUTES.PERIOD > 0) {
    // Periodic process
    if(DELAY_TIME >=
       l2_scheduler_state.process_table[PROCESS_ID].
       process_status.ATTRIBUTES.PERIOD) {
      // Error, as required by the standard (although I don't see the
      // reason).
      *RETURN_CODE = INVALID_PARAM ;
      return ;
    }
  }
  //
  switch(l1_partition_interface.partition_status.OPERATING_MODE) {
  case COLD_START:
  case WARM_START:
    // If in non-preemptive mode, I can directly start the
    // actual handler here.
    *RETURN_CODE = handler_delayed_start(PROCESS_ID,-1,DELAY_TIME) ; ;
    break ;
  case NORMAL:
    // In preemptive mode, only a process can request
    // DELAYED_START on another (not the kernel). And the request
    // has to go through the L1 kernel sequencing service before
    // the L2 kernel calls the handler.
    {
      RETURN_CODE_TYPE rc ;
      PROCESS_ID_TYPE my_id ;
      GET_MY_ID(&my_id,&rc) ;
      // Sanity check.
      if(rc != NO_ERROR) {
	fatal_error("DELAYED_START:Inconsistent scheduler state.") ;
      }
      *RETURN_CODE = svc_l2_request_delayed_start(PROCESS_ID,my_id,DELAY_TIME) ;
    }
    break ;
  default:
    *RETURN_CODE = INVALID_MODE ;
    break ;
  }
}

void GET_MY_ID(/*out*/ PROCESS_ID_TYPE *PROCESS_ID,
	       /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  // At this point, the running process should be the one pointed to
  // by L2_process_queue_head. 
  if(l2_scheduler_state.running_process < 0) {
    *RETURN_CODE = INVALID_MODE ;
  } else if(l2_scheduler_state.running_process >= MAX_PROCESS_NUMBER) {
    *RETURN_CODE = INVALID_MODE ;
  } else if(!l2_scheduler_state.process_table[l2_scheduler_state.
					      running_process].record_used){
    *RETURN_CODE = INVALID_MODE ;
  } else {
    *PROCESS_ID = l2_scheduler_state.running_process ;
    *RETURN_CODE = NO_ERROR ;
  }
}

void GET_PROCESS_STATUS (/*in */ PROCESS_ID_TYPE PROCESS_ID,
			 /*out*/ PROCESS_STATUS_TYPE *PROCESS_STATUS,
			 /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  if((PROCESS_ID < 0)||(PROCESS_ID >= MAX_PROCESS_NUMBER)) {
    *RETURN_CODE = INVALID_PARAM ;
  } else if(!l2_scheduler_state.process_table[PROCESS_ID].record_used) {
    *RETURN_CODE = INVALID_PARAM ;
  } else {
    *PROCESS_STATUS =
      l2_scheduler_state.process_table[PROCESS_ID].process_status ;
    *RETURN_CODE = NO_ERROR ;
  }
}

void PERIODIC_WAIT(/*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  PROCESS_ID_TYPE pid ;
  RETURN_CODE_TYPE rc ;
  GET_MY_ID(&pid,&rc) ;
  if(rc != NO_ERROR) {
    fatal_error("PERIODIC_WAIT: GET_MY_ID returned error.") ;
  }
  if(l1_partition_interface.partition_status.OPERATING_MODE != NORMAL) {
    // Not preemptive mode
    *RETURN_CODE = INVALID_MODE ;
    return ;
  }
  if(l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.PERIOD
     <= 0) {
    // Process is not periodic
    *RETURN_CODE = INVALID_MODE ;
    return ;
  }
  *RETURN_CODE = svc_l2_request_periodic_wait() ;
}

void START (/*in */ PROCESS_ID_TYPE PROCESS_ID,
	    /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  DELAYED_START(PROCESS_ID,0,RETURN_CODE) ;
}

/* This routine should **not** be called from within
   the L2 scheduler, because GET_MY_ID may return a 
   value, and therefore modify the status of the running
   process. 
   One should directly use instead 
   svc_l2_request_stop(PROCESS_ID,-1) */
void STOP (/*in */ PROCESS_ID_TYPE PROCESS_ID,
	   /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  if(!l2_scheduler_state.process_table[PROCESS_ID].record_used) {
    *RETURN_CODE = INVALID_PARAM ;
    return ;
  }
  if(l2_scheduler_state.process_table[PROCESS_ID].
     process_status.PROCESS_STATE
     == DORMANT) {
    *RETURN_CODE = NO_ACTION ;
    return ;
  }
  switch(l1_partition_interface.partition_status.OPERATING_MODE) {
  case COLD_START:
  case WARM_START:
    handler_stop_process(PROCESS_ID,-1) ;
    *RETURN_CODE = NO_ERROR ;
    break ;
  case NORMAL:
    {
      RETURN_CODE_TYPE rc ;
      PROCESS_ID_TYPE my_id ;
      GET_MY_ID(&my_id,&rc) ;
      // This second case is for when the call comes from
      // the kernel itself in NORMAL mode.
      if(rc != NO_ERROR) fatal_error("Corrupted data.\n") ;
      *RETURN_CODE = svc_l2_request_stop(PROCESS_ID,my_id) ;
    }
    break ;
  default:
    *RETURN_CODE = INVALID_MODE ;
    break ;
  }
}

void STOP_SELF() {
  RETURN_CODE_TYPE rc ;
  PROCESS_ID_TYPE  pid ;
  GET_MY_ID(&pid,&rc) ;
  if(rc == NO_ERROR) {
    STOP(pid,&rc) ;
  }
}

/*--------------------------------------------------*/
/*--------------------------------------------------*/

void SUSPEND_SELF (/*in */ SYSTEM_TIME_TYPE TIME_OUT,
		   /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  if(l1_partition_interface.partition_status.OPERATING_MODE
     !=NORMAL) {
    fatal_error("SUSPEND_SELF:Called in mode != NORMAL.\n") ;
  }
  PROCESS_ID_TYPE  pid ;
  GET_MY_ID(&pid,RETURN_CODE) ;
  if(*RETURN_CODE != NO_ERROR) fatal_error("SUSPEND_SELF:Corrupted data.\n") ;
  *RETURN_CODE = svc_l2_request_suspend(pid,pid,TIME_OUT) ;
}

void SUSPEND (/*in */ PROCESS_ID_TYPE PROCESS_ID,
	      /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  switch(l1_partition_interface.partition_status.OPERATING_MODE) {
  case COLD_START:
  case WARM_START:
    *RETURN_CODE = handler_suspend(PROCESS_ID,-1,INFINITE_TIME_VALUE) ;
    break ;
  case NORMAL:
    {
      PROCESS_ID_TYPE  pid ;
      GET_MY_ID(&pid,RETURN_CODE) ;
      if(*RETURN_CODE != NO_ERROR) fatal_error("SUSPEND:Corrupted data.\n") ;
      *RETURN_CODE =
	svc_l2_request_suspend(PROCESS_ID,pid,INFINITE_TIME_VALUE) ;
    }
    break ;
  default:
    *RETURN_CODE = INVALID_MODE ;
    break ;
  }
}

void RESUME (/*in */ PROCESS_ID_TYPE PROCESS_ID,
	     /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  switch(l1_partition_interface.partition_status.OPERATING_MODE) {
  case COLD_START:
  case WARM_START:
    *RETURN_CODE = handler_resume(PROCESS_ID,-1) ;
    break ;
  case NORMAL:
    {
      PROCESS_ID_TYPE  pid ;
      GET_MY_ID(&pid,RETURN_CODE) ;
      if(*RETURN_CODE != NO_ERROR) fatal_error("RESUME:Corrupted data.\n") ;
      *RETURN_CODE = svc_l2_request_resume(PROCESS_ID,pid) ;
    }
    break ;
  default:
    *RETURN_CODE = INVALID_MODE ;
    break ;
  }
}


