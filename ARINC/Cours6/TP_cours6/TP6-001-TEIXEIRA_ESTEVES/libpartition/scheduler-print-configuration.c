
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
#include <librpi/mmu.h>              // Print page table function
#include <librpi/debug.h>            // debug_printf
#include <libpartition/scheduler.h>  // For the data structure to print.
#include <kernel/l1-l2-interface.h>      // For the event types.
#include <libpartition/svc-l2-request.h> // For the l2 event types.
#include <librpi/indent.h>

const char* event_type2string(enum L2_event_type et) {
  switch(et) {
  case NO_EVENT: return "NO_EVENT";
  case KERNEL_TICK_EVENT: return "KERNEL_TICK_EVENT";
  case PARTITION_L2_EVENT: return "PARTITION_L2_EVENT";
  default: return "**ERROR**" ;
  }
}

const char* l2_event_type2string(enum L2RequestType et) {
  switch(et) {
  case L2_REQ_UNUSED: return "L2_REQ_UNUSED" ;
  case L2_REQ_PERIODIC_WAIT: return "L2_REQ_PERIODIC_WAIT" ;
  case L2_REQ_STOP: return "L2_REQ_STOP" ;
  case L2_REQ_DELAYED_START: return "L2_REQ_DELAYED_START" ;
  default: return "**ERROR**" ;
  }
}

const char* partition_mode2string(OPERATING_MODE_TYPE om) {
  switch(om) {
  case IDLE: return "IDLE" ;
  case COLD_START: return "COLD_START" ;
  case WARM_START: return  "WARM_START" ;
  case NORMAL: return "NORMAL" ;
  default: return "**ERROR**" ;
  }
}

const char* process_state2string(PROCESS_STATE_TYPE ps,
				 enum ProcessWaitingCause  wc) {
  switch(ps) {
  case DORMANT: return "DORMANT" ;
  case READY: return "READY" ;
  case RUNNING: return "RUNNING" ;
  case WAITING:
    switch(wc) {
    case WaitSuspended: return "WAIT(SUSP)" ;
    case WaitTimer: return "WAIT(TIMER)" ;
    case WaitPeriodicTimer: return "WAIT(PERIOD)" ;
    case WaitNormalMode: return "WAIT(NORMAL)" ;
    case WaitPartitionPeriodStart: return "WAIT(PPS)" ;
    default: return "WAIT(**ERROR**)" ;
    }
  default: return "**ERROR**" ;
  }
}

const char* deadline_type2string(DEADLINE_TYPE dt) {
  switch(dt) {
  case SOFT: return "SOFT" ;
  case HARD: return "HARD" ;
  default: return "**ERROR**" ;
  }
}

void PrintEventQueue() {
  int i ;
  debug_printf("event queue: first:%d last:%d entries:\n",
	       l1_partition_interface.eq.first,
	       l1_partition_interface.eq.last) ;
  indent_increment_depth() ;
  for(i=0;i<EVENT_QUEUE_SIZE;i++) {
    switch(l1_partition_interface.eq.events[i].type) {
    case NO_EVENT:
      debug_printf("[NO_EVENT]\n") ;
      break ;
    case KERNEL_TICK_EVENT:
      debug_printf("[KERNEL_TICK_EVENT: %s ticks_since_last: %d]\n",
		   (l1_partition_interface.eq.events[i].d.kernel_tick.
		    is_partition_periodic_start?"PPS":""),
		   l1_partition_interface.eq.events[i].d.kernel_tick.
		   system_ticks_since_last_partition_tick);
      break ;
    case PARTITION_L2_EVENT:
      {
	struct L2Request* l2_req =
	  (struct L2Request*)l1_partition_interface.eq.
	  events[i].d.partition_l2.req_data_ptr ;
	debug_printf("[PARTITION_L2_EVENT:%s tpid:%d cpid:%d delay:%d]\n",
		     l2_event_type2string(l2_req->type),
		     l2_req->target_pid,
		     l2_req->caller_pid,
		     l2_req->delay) ;
      }
      break ;
    default:
      debug_printf("[**ERROR**]\n");
      break ;
    }
    /*
    debug_printf("[type:%s is_pps:%u system_ticks_since_last_partition_tick:%u]\n",
		 event_type2string(l1_partition_interface.eq.events[i].type),
		 l1_partition_interface.eq.events[i].d.kernel_tick.
		 is_partition_periodic_start,
		 l1_partition_interface.eq.events[i].d.kernel_tick.
		 system_ticks_since_last_partition_tick) ;
    */
  }
  indent_decrement_depth() ;
}

void PrintProcessL2Info(PROCESS_ID_TYPE pid) {
  if(!l2_scheduler_state.process_table[pid].record_used) {
    debug_printf("Unused process record for id %d\n",pid) ;
  } else {
    debug_printf("Process[%d] state:%s current prio:%d deadline:%d\n"
		 "Period:%x TimeCap:%d EntryPoint:%x StackSize:%x\n"
		 "BasePrio:%d DeadlineType:%s ProcessName:%s \n"
		 "StackBase:%x\n",
		 pid,
		 process_state2string(l2_scheduler_state.
				       process_table[pid].process_status.
				       PROCESS_STATE,
				       l2_scheduler_state.
				       process_table[pid].waiting_cause),
		 l2_scheduler_state.process_table[pid].process_status.CURRENT_PRIORITY,
		 l2_scheduler_state.process_table[pid].process_status.DEADLINE_TIME,
		 l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.PERIOD ,
		 l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.TIME_CAPACITY ,
		 l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.ENTRY_POINT ,
		 l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.STACK_SIZE ,
		 l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.BASE_PRIORITY ,
		 deadline_type2string(l2_scheduler_state.process_table[pid].
				      process_status.ATTRIBUTES.DEADLINE) ,
		 l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.NAME,
		 l2_scheduler_state.process_table[pid].process_stack_base) ;
    indent_increment_depth() ;
    if((l2_scheduler_state.process_table[pid].process_status.PROCESS_STATE == WAITING)&&
       (l2_scheduler_state.process_table[pid].waiting_cause == WaitTimer)) {
      // The process is waiting for a timer.
      debug_printf("timer_timeout:%d\n",
		   l2_scheduler_state.process_table[pid].timer_timeout) ;
    }
    if(l2_scheduler_state.process_table[pid].process_status.ATTRIBUTES.PERIOD !=0){
      // The process is periodic.
      debug_printf("periodic process next_release_point:%d\n",
		   l2_scheduler_state.process_table[pid].next_release_point) ;
    }
    if((l2_scheduler_state.process_table[pid].process_status.PROCESS_STATE == READY)||
       (l2_scheduler_state.process_table[pid].process_status.PROCESS_STATE == RUNNING)) {
      // The process is in queue
      debug_printf("next in queue: %d\n",
		   l2_scheduler_state.process_table[pid].next_in_queue) ;
    }
    indent_decrement_depth() ;
  }
}

		  
// ATTENTION: multiple values I don't use and/or don't really understand
// are not printed here. Among them:
// The partition period, duration, lock level, and start condition.
void PrintL2SchedulerState() {
  indent_increment_depth() ;
  // First, print the interface.
  debug_printf("l1_partition_interface:\n") ;
  indent_increment_depth() ;
  debug_printf("%s mode:%s tick_length:0x%x\n"
	       "process context saved:%s:\n",
	       (l1_partition_interface.is_initialized?"initialized":"non-initialized"),
	       partition_mode2string(l1_partition_interface.
				     partition_status.OPERATING_MODE),
	       l1_partition_interface.tick_length,
	       (l1_partition_interface.process_context_saved?"Y":"N")) ;
  if(l1_partition_interface.process_context_saved) {
    indent_increment_depth() ;
    PrintRegisterSet(&l1_partition_interface.process_context) ;
    indent_decrement_depth() ;
  }  
  PrintEventQueue() ;
  indent_decrement_depth() ;
  // Second, print the L2SchedulerState structure
  debug_printf("l2_scheduler_state:\n") ;
  indent_increment_depth() ;
  debug_printf("heap_b:%x heap_s:%x stack_b:%x free_stack:%x\n"
	       "running process: %d max process number: %d\n"
	       "used process records:\n",
	       l2_scheduler_state.mmap.heap_base,
	       l2_scheduler_state.mmap.heap_size,
	       l2_scheduler_state.mmap.stack_base,
	       l2_scheduler_state.mmap.free_process_stack,
	       l2_scheduler_state.running_process,
	       MAX_PROCESS_NUMBER) ;
  indent_increment_depth() ;
  {
    PROCESS_ID_TYPE pid ;
    for(pid=0;pid<MAX_PROCESS_NUMBER;pid++) {
      if(l2_scheduler_state.process_table[pid].record_used) {
	PrintProcessL2Info(pid) ;
      }
    }
  }
  indent_decrement_depth() ;
  indent_decrement_depth() ;
  indent_decrement_depth() ;
}
