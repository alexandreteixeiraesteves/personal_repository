
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
#ifndef L1_L2_INTERFACE_H
#define L1_L2_INTERFACE_H

#include <libc/stdint.h>      // For uint32_t
#include <arinc653/types.h>   // For PARTITION_STATUS_TYPE
#include <librpi/registers.h> // For struct FullRegisterSet

//========================================================================
// Event queue definitions.
// The L1 and L2 schedulers interact through an event queue. All
// requests to the L2 scheduler are enqueued here **by the L1 kernel**
// (process requests are done through a service).  The L1 scheduler
// enqueues them, and then the L2 scheduler simply processes them one
// at a time. Each request is processed in its own context
// (e.g. time), which ensures logical consistency (logical time is
// respected).
//------------------------------------------------------------------------
// Event queue sizing.  Two events should be enough for the current OS
// implementation. This queue should allow the buffering of the
// incoming events. If the only IRQ comes from the timer, then a queue
// of depth 3 should be enough, because only 2 events may be stored at
// the same time (one internal event and one IRQ interrupting it).
#define EVENT_QUEUE_SIZE 3

// Decoding L2 events. 
enum L2_event_type {
  // Default value denoting no event. It is used internally
  // in the L1 scheduler and to difference legitimate events
  // from errors.
  NO_EVENT = 0,
  // Tick event coming from the L1 kernel
  KERNEL_TICK_EVENT = 1,
  // Request coming from the partition itself (process or even the
  // L2 scheduler that needs to timestamp smth.).
  PARTITION_L2_EVENT = 2,
} ;

// One event in the event queue.
struct L2_event {
  // Actual request type
  enum L2_event_type type ;
  // Decoder union
  union {
    // For tick requests
    struct {
      uint32_t is_partition_periodic_start ;
      uint32_t system_ticks_since_last_partition_tick ;      
    } kernel_tick ;
    // For L2 requests
    struct {
      // This pointer is only interpreted in partition code.
      void* req_data_ptr ;
    } partition_l2 ;
  } d ; 
} ;

struct L2_event_queue {
  struct L2_event events[EVENT_QUEUE_SIZE] ;
  // Modified only by the L2 scheduler, which dequeues. Points to the
  // first element  in the queue.
  uint32_t first ;
  // Modified only by the L1 scheduler, which enqueues. Points to the
  // element just after the last element in the queue.
  uint32_t last ;
} ;

//========================================================================



//========================================================================
// The partition interface
// Data structure used to exchange data between
// the partitions and the kernel. This data structure
// is stored in the memory space of the partition,
// because partition code cannot address
// kernel memory. It must be reset at the beginning of execution and
// at partition errors
//
// In addition to this, the only means for data exchange 
// is through registers during SVC calls (of course, the kernel can 
// address memory everywhere, e.g. allowing to print 
// strings stored in user space).
struct L1PartitionInterface {
  //-------------------------------------------------------
  // I need from the kernel some real-time information,
  // such as the length of the tick.
  uint32_t tick_length ;
  
  //-------------------------------------------------------
  // Initialization flag, set to false at ELF load or by
  // bzeroing the .bss of the partition.
  uint32_t is_initialized ;
  // Arinc 653 partition status
  PARTITION_STATUS_TYPE partition_status ;

  //-------------------------------------------------------
  // Saved process status.
  // Whenever in_a_process is true when a context is saved,
  // the context is saved here, and the process_context_saved
  // indicator is set to 1.
  //
  // I can optimize this out later, but for now I want to keep
  // things simple, even if I have to copy the context twice.
  uint32_t process_context_saved ;
  struct FullRegisterSet process_context ;

  
  //-------------------------------------------------------
  // Event queue.
  struct L2_event_queue eq ;
} ;
//========================================================================

#endif
