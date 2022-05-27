
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
#include <libc/util.h>                   // For increment_modulo
#include <libc/string.h>                 // For bzero
#include <librpi/debug.h>                // For debug_printf
#include <arinc653/process.h>            // For PROCESS_ID_TYPE
#include <libpartition/svc-call.h>       // For svc_sched_restore_process_context
#include <libpartition/svc-l2-request.h> // For L2Request
#include <libpartition/scheduler.h>      // For the scheduler data structs
#include <libc/stdlib.h>
#include <librpi/indent.h>

void set_running_process() {
  // This function is not the one choosing which process is to
  // run next (the scheduling algorithm). It merely marks as
  // RUNNING the process in front of the process queue, if the
  // queue is not empty.
  // The actual implementation of the scheduling policy is
  // in the L2 request handlers.
  if(l2_scheduler_state.running_process >= 0) {
    // The head of the process queue is not empty (running_process >=0).
    l2_scheduler_state.process_table[l2_scheduler_state.running_process].
      process_status.PROCESS_STATE = RUNNING ;
    // Sanity check
    if(!l2_scheduler_state.process_table[l2_scheduler_state.running_process].
       record_used) {
      // If this sort of corruption occurs, the partition must be stopped.
      fatal_error("set_running_process: running process %d record not used. "
		  "Enter IDLE mode.\n") ;
    }
  }
}  

void L2_process_l2_request(struct L2Request* l2_request) {
  //debug_printf("L2_process_l2_request:%d",l2_request->type) ;
  switch(l2_request->type) {
  case L2_REQ_PERIODIC_WAIT:
    handler_partition_l2_periodic_wait() ;
    break ;
  case L2_REQ_STOP:
    handler_stop_process(l2_request->target_pid,
			 l2_request->caller_pid) ;
    break ;
  case L2_REQ_DELAYED_START:
    handler_delayed_start(l2_request->target_pid,
			  l2_request->caller_pid,
			  l2_request->delay) ;
    break ;
  case L2_REQ_SUSPEND:
    handler_suspend(l2_request->target_pid,
		    l2_request->caller_pid,
		    l2_request->delay) ;
    break ;
  case L2_REQ_RESUME:
    handler_resume(l2_request->target_pid,
		   l2_request->caller_pid) ;
    break ;
  case L2_REQ_TIMED_WAIT:
    handler_timed_wait(l2_request->delay) ;
    break ;
  default:
    fatal_error("L2_process_l2_request:Bad request type\n") ;
  }
}

// Decode a single request and update the state accordingly
void L2_process_single_event(struct L2_event* event_ptr) {
  switch(event_ptr->type) {
  case KERNEL_TICK_EVENT:
    handler_tick_event(event_ptr->d.kernel_tick.
		       system_ticks_since_last_partition_tick,
		       event_ptr->d.kernel_tick.
		       is_partition_periodic_start) ;
    break ;
  case PARTITION_L2_EVENT:
    L2_process_l2_request(event_ptr->d.partition_l2.req_data_ptr) ;
    break ;
  default:
    fatal_error("L2_process_single_event: Bad event type.\n") ;
  }
}
  
void L2_normal_mode_scheduler() {
  // Actions that need to be taken upon entering normal mode.
  handler_enter_normal_mode() ;
  // Loop forever treating events. If a partition mode change
  // occurs, it's treated at L1 scheduler level.
  for(;;) {
    //debug_printf("L2_normal_mode_schedule: scheduler state:\n") ;
    //indent_increment_depth() ;
    //PrintL2SchedulerState() ;
    //indent_decrement_depth() ;
      
    // If a process state is saved, save it to the active
    // process state.
    if(l1_partition_interface.process_context_saved) {
      l2_scheduler_state.
	process_table[l2_scheduler_state.running_process].
	context =
	l1_partition_interface.process_context ;
      // Reset the indicator.
      l1_partition_interface.process_context_saved = 0 ;
    }

    // Purge all events in the queue before giving control to
    // a process. The purge is not to be (functionally)
    // interrupted by the L1 scheduler (control must be given
    // back, not to the start of the L2_step_function).
    while(l1_partition_interface.eq.first !=
	  l1_partition_interface.eq.last) {
      // State update for one event
      L2_process_single_event(&l1_partition_interface.eq.
			      events[l1_partition_interface.eq.
				     first]) ;
      // Increment the pointer modulo queue size
      l1_partition_interface.eq.first =
	increment_modulo(l1_partition_interface.eq.first,
			 EVENT_QUEUE_SIZE) ;
    }
    
    // The active process is set 

    // Now change, if needed, the active process
    set_running_process() ;
    //PrintL2SchedulerState() ;
    
    //
    // debug_puts("L2_step_function: before context restore:\n") ;
    // If some process is running, give control to it.
    if(l2_scheduler_state.running_process >= 0) {
      //debug_printf("Active process: %d register set:\n",
      //		   l2_scheduler_state.running_process) ;
      //PrintRegisterSet(&l2_scheduler_state.
      //		       process_table[l2_scheduler_state.
      //				     running_process].context) ;      
      svc_sched_restore_process_context(&l2_scheduler_state.
					process_table[l2_scheduler_state.
						      running_process].context) ;
    }	
  }
}
