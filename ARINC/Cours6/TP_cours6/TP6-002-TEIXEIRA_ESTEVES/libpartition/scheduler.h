
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
#ifndef L2_SCHEDULER_H
#define L2_SCHEDULER_H

#include <libc/stdint.h>            // Basic types
#include <librpi/registers.h>       // For FullRegisterSet
#include <arinc653/types.h>         // ARINC 653-specific
#include <arinc653/process.h>       // ARINC 653-specific
#include <kernel/scheduler.h>       // L1 interface struct
#include <libpartition/constants.h> // For init constants

//==========================================================
// L2 scheduler data structures
//==========================================================
//

enum ProcessWaitingCause {
  //--------------------------------------------------------
  // Configured waiting. Only regular scheduling operations
  // are needed.
  //
  // Suspended undefinitely, awaiting an explicit resume
  // (not waiting for some event).
  WaitSuspended            = 0x01,
  // Waiting for a timer timeout. This is the case for both
  // periodic and aperiodic processes waiting for
  // a TIMED_WAIT. It does not cover PERIODIC_WAIT. This is
  // because TIMED_WAIT and PERIODIC_WAIT need to use
  // different timers. The timer here is timer_value defined
  // below. Waiting is over when this variable becomes <= 0.
  WaitTimer                = 0x02,
  // Distance to the next periodic process start point.
  // Only for periodic processes. It uses the
  // next_release_point timer variable.
  WaitPeriodicTimer        = 0x03,

  //--------------------------------------------------------
  // Unconfigured waiting. When the event arrives, more
  // config is needed.
  //
  // Waiting for NORMAL mode. These processes should have
  // various dates recomputed upon entering NORMAL mode.
  // This wait is for aperiodic processes only.
  // Cannot have other sources of waiting (decoding stops).
  WaitNormalMode           = 0x10,
  // Waiting for the first partition period start window
  // in NORMAL mode. These processes should have
  // various dates recomputed upon getting the first
  // PPS in NORMAL mode.
  // This wait is for periodic processes only. During this
  // sort of wait, the process is not yet normally started,
  // and other events are not taken into account.
  WaitPartitionPeriodStart = 0x20,
} ;

// process status information
struct L2_process_record {
  //-----------------------------------------------------------
  // Basic data:
  // Is this process record used? The remainder of the
  // record is undefined if this is false.
  int32_t                     record_used ;
  // ARINC 653 process status
  PROCESS_STATUS_TYPE         process_status ;
  // Memory configuration: stack and area to save context.
  // Stack base does not change after configuration
  uint32_t                    process_stack_base ;
  struct FullRegisterSet      context ;

  //-----------------------------------------------------------
  // Internal scheduler data:
  // If the process is in WAITING state, this encodes the
  // waiting cause.
  enum ProcessWaitingCause  waiting_cause ;

  // Deadline is not stored here, but in the ARINC 653
  // process status. It's stored in ticks remaining to pass.
  // This must be less or equal to the period of the task.
  // Deadline miss is when deadline becomes negative.

  // Timer timeout, in ticks remaining to pass. This is
  // only for TIMED_WAIT (attention, this one can be used
  // also with periodic processes).
  int32_t                     timer_timeout ;

  // Only for periodic processes. This value tells the distance
  // in ticks to the next release point.
  // Upon periodic process creation, this value is set to the
  // delay (0 for START, possibly more for DELAYED_START).
  // Upon reaching the partition periodic start, it is set to
  // the distance to the next triggering point.
  // When it becomes <=0 the process is released and
  // PERIOD is added to next_release_point.
  // This value can become negative when the triggering date
  // falls inside another process' partition. In this case, the
  // negative offset must be taken into account to remain
  // periodic.
  // This value can be larger than the MTF, for processes with
  // period larger than the MTF.
  int32_t                     next_release_point ;

  // Process READY/RUNNING queue stuff
  PROCESS_ID_TYPE next_in_queue ;
  
} ;

//==========================================================
// Scheduler state data structure
//==========================================================
struct L2SchedulerState {
  // L2 memory configuration, local part (in addition to
  // the part provided by the kernel in the interface).
  struct LocalMemoryConfiguration {
    // The limits of the heap
    uint32_t heap_base ;
    uint32_t heap_size ;
    // The partition stack base
    uint32_t stack_base ;
    // This variable holds the pointer to free stack, to be
    // allocated upon process creation.
    uint32_t free_process_stack ;
  } mmap ;

  // Process entries
  struct L2_process_record process_table[MAX_PROCESS_NUMBER] ;
  
  // Head of the queue containing running and ready
  // processes.
  PROCESS_ID_TYPE running_process ;
} ;

//==========================================================
// Internal scheduler functions.
//==========================================================

// The entry point of the scheduler. Nor really
// useful to define it here, but why not.
void L2_entry_point(void) ;

// Scheduler functions that handle various processing
// modes, called directly by L2_entry_point.
void L2_start_mode(void) ;
void L2_normal_mode_scheduler(void) ;

// Event queue management routines, called at various
// places in the scheduler functions.
void insert_into_process_queue(PROCESS_ID_TYPE pid) ;
void remove_from_process_queue(PROCESS_ID_TYPE pid) ;

// Handlers for various events.
// Called from the L2 normal mode scheduler only.
void handler_enter_normal_mode(void) ;
void handler_tick_event(uint32_t system_ticks_since_last_partition_tick,
			uint32_t is_partition_periodic_start) ;
void handler_partition_l2_periodic_wait(void) ;
// Called from the L2 normal mode scheduler and from
// ARINC 653 routines, when they are in elaboration
// mode.
void handler_stop_process(PROCESS_ID_TYPE target_pid,
			  PROCESS_ID_TYPE caller_pid) ;
RETURN_CODE_TYPE handler_delayed_start(PROCESS_ID_TYPE target_pid,
				       PROCESS_ID_TYPE caller_pid,
				       SYSTEM_TIME_TYPE delay) ;
RETURN_CODE_TYPE handler_suspend(PROCESS_ID_TYPE target_pid,
				 PROCESS_ID_TYPE caller_pid,
				 SYSTEM_TIME_TYPE delay) ;
RETURN_CODE_TYPE handler_resume(PROCESS_ID_TYPE target_pid,
				PROCESS_ID_TYPE caller_pid) ;
void handler_timed_wait(SYSTEM_TIME_TYPE delay) ;
// Called when a process terminates execution (it is
// set in the LR register, in the initial context).
void handler_process_exit(void) ;

// Print the l2 scheduler state, including the interface.
void PrintL2SchedulerState(void) ;

//==========================================================
// Statically-allocated data structure (from scheduler.c).
//==========================================================
extern struct L1PartitionInterface l1_partition_interface ;
extern struct L2SchedulerState l2_scheduler_state ;

//==========================================================
// Defined in partition-specific code, this is the
// entry point of the elaboration code with
// ARINC 653/APEX API. "main process" is the name used
// for this code in the standard.
//==========================================================
extern void main_process(void) ;


#endif // SCHEDULER_H
