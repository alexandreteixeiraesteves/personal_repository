
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
#include <librpi/debug.h>                // For debug_printf
#include <arinc653/process.h>            // For PROCESS_ID_TYPE
#include <libpartition/scheduler.h>      // For the scheduler data structs


// Remove a process from the READY/RUNNING queue.
void insert_into_process_queue(PROCESS_ID_TYPE pid) {
  struct L2_process_record * process =
    &l2_scheduler_state.process_table[pid] ;
  
  if(l2_scheduler_state.running_process < 0) {
    // Queue is currently empty.
    l2_scheduler_state.running_process = pid ;
    process->next_in_queue = -1 ;
    return ;
  } else {
    if(l2_scheduler_state.
       process_table[l2_scheduler_state.running_process].
       process_status.CURRENT_PRIORITY <
       process->process_status.CURRENT_PRIORITY) {
      // The newly ready process is most prioritary, it takes the
      // place of running process (insert at the front of the queue).
      process->next_in_queue = l2_scheduler_state.running_process ;
      l2_scheduler_state.running_process = pid ;
    } else {
      PROCESS_ID_TYPE tmp_pid = l2_scheduler_state.running_process ;
      struct L2_process_record * tmp_process =
	&l2_scheduler_state.process_table[tmp_pid] ;
      int done = 0 ;
      do {
	if(tmp_process->next_in_queue < 0) {
	  // Insert at the end of the queue
	  tmp_process->next_in_queue = pid ;
	  process->next_in_queue = -1 ;
	  done = 1 ;
	} else {
	  // There is a next process
	  if(l2_scheduler_state.
	     process_table[tmp_process->next_in_queue].
	     process_status.CURRENT_PRIORITY <
	     process->process_status.CURRENT_PRIORITY) {
	    // Insert in the middle of the queue
	    process->next_in_queue = tmp_process->next_in_queue ;
	    tmp_process->next_in_queue = pid ;
	    done = 1 ;
	  } else {
	    tmp_pid = tmp_process->next_in_queue ;
	    tmp_process = &l2_scheduler_state.process_table[tmp_pid] ;
	  }
	}
      } while (!done) ;
    }
  }
}

// Remove a process from the READY/RUNNING queue.
void remove_from_process_queue(PROCESS_ID_TYPE pid) {
  if(l2_scheduler_state.running_process < 0) {
    debug_printf("remove_from_process_queue: error: empty queue !\n") ;
  } else {
    if(pid == l2_scheduler_state.running_process) {
      // Remove from front.
      l2_scheduler_state.running_process =
	l2_scheduler_state.process_table[pid].next_in_queue ;
      l2_scheduler_state.process_table[pid].next_in_queue = -1 ;
    } else {
      // The other cases.
      PROCESS_ID_TYPE tmp_pid = l2_scheduler_state.running_process ;
      PROCESS_ID_TYPE next_pid =
	l2_scheduler_state.process_table[tmp_pid].next_in_queue ;
      while((next_pid >= 0) && (next_pid != pid)) {
	tmp_pid = next_pid ;
	next_pid = l2_scheduler_state.process_table[tmp_pid].next_in_queue ;
      }
      if(next_pid < 0) {
	debug_printf("remove_from_process_queue: error: pid not in queue !\n") ;
      } else {
	// Found it. Remove it.
	l2_scheduler_state.process_table[tmp_pid].next_in_queue =
	  l2_scheduler_state.process_table[next_pid].next_in_queue ;
	l2_scheduler_state.process_table[next_pid].next_in_queue = -1 ;
      }
    }
  }
}

