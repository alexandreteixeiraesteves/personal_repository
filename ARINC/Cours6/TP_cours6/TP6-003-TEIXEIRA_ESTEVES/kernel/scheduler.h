
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
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <libc/stdint.h>            // For uint32_t
#include <librpi/registers.h>       // For FullRegisterSet
#include <librpi/mmu.h>             // For the DACR
#include <arinc653/partition.h>     // For the ARINC 653 data types
#include <kernel/l1-l2-interface.h> // For the interface with L2 scheduler
                                    // defined separately.

// Implementation-dependent constants.
// The maximal number of partitions must include the
// system partition and the application partitions.
// I choose 14, which should allow me to use a single
// page table and only change memory domain permissions
// in the MMU. This means I can use 13 application
// partitions, which seems enough in practice.
#define MAX_PARTITION_NUMBER 14
// For the number of windows, I set 64
#define MAX_WINDOW_NUMBER    64
// Partition 0 is the system partition.
#define SYSTEM_PARTITION_ID 0

// Maximal number of ports in a partition
#define MAX_PARTITION_PORTS 32
#define MAX_CHANNELS 32


//========================================================================
// L1 internal data structures, no L2 access.
//------------------------------------------------------------------------

struct PartitionPort {
  enum {
    IN,
    OUT
  } direction ;
  char name [32] ;
} ;

#define CHANNEL_BUFFER_SIZE 3

struct QueuingMessage {
  uint32_t size ;
  unsigned char* data ;
} ;

struct Channel {
  uint32_t source_partition_id ;
  uint32_t source_port_id ;
  uint32_t dest_partition_id ;
  uint32_t dest_port_id ;
  uint32_t max_msg_size ;
  // Channel buffer
  struct QueuingMessage data_buffer[CHANNEL_BUFFER_SIZE] ;
  uint32_t first ;
  uint32_t last ;
} ;

// Data structure storing the characteristics of
// a partition allowing memory configuration and
// partition init and re-init. This data is only
// available to the L1 scheduler. The L2 scheduler
// does not need such information, it gets it from
// the calling context.
struct L1PartitionMemoryConfiguration{
  //------------------------------------------------------
  // Only for the user partitions.
  // 
  // Limits of the partition memory, fixed by config file.
  // Only used for the user partitions. 0 for the system
  // partition.
  uint32_t memory_base ; 
  uint32_t memory_size ; // In octets
  // The position and size of the bss. Only used for the
  // user partitions. 0 for the system partition.
  uint32_t bss_base ;
  uint32_t bss_size ; // In octets

  //------------------------------------------------------
  // For all partitions, including the system one.
  //   
  // The position of the stack base.
  // It is set by the L1 scheduler and then used
  // for init by the L2_init_function and by the
  // scheduler step routine.
  // Currently, this is the memory end.
  uint32_t stack_base ;
  // The single entry point. Currently, equal to memory_base.
  uint32_t entry_point ;
  // The position of the interface, which is currently
  // at the very beginning of the .bss
  struct L1PartitionInterface* interface ;
} ;

// This is the record stored for each partition by the
// L1 scheduler. The information allows the L1 scheduler
// to configure and run it.
struct L1SchedulerPartitionState{
  // Context saved for the L2 scheduler, which gets
  // control back each time it is interrupted.
  struct FullRegisterSet l2_scheduler_context ;
  // Memory map (private to the kernel). Its value does not
  // change after load, allowing re-init after a fault.
  // However, the interface it contains points to the
  // place where the internal partition state is held.
  struct L1PartitionMemoryConfiguration mmap ;
  // Initialization value for the partition state structure.
  // It does not change after load, allowing re-initialization
  // after a fault.
  PARTITION_STATUS_TYPE init_partition_status ;
  // The domain access control register gives the permission
  // mask with which the partition should be executed.
  // The kernel will have all permissions, whereas the
  // partitions can only access their memory area
  // (and temporarily the peripherals).
  union DomainAccessControlRegister dacr ;
  // Boolean determining whether the currently-executing code
  // belongs to a process, or not (init function, including
  // elaboration code, and step function, are not process
  // code). It is set to 1 when a process is started through
  // kernel service. It is set to 0 when scheduler code is
  // given control.
  uint32_t in_a_process ;
  // This value is updated at each tick of the system
  // clock by the L1 scheduler. It is set to 0 as part
  // of configuration (and reset if the partition is reset).
  // It is used to initialize the time of the 
  // - By the L1 scheduler when the partition is reset
  //   (including the initial configuration). This is
  //   done by L1_reset_partition, by the 
  // - By the L2 scheduler each time this one processes
  //   a timer event.
  uint32_t system_ticks_since_last_partition_tick ;
  
  //----------------------------------------------------
  // Ports -- very simple code.
  uint32_t number_of_ports ;
  struct PartitionPort ports[MAX_PARTITION_PORTS] ;
};

// MTF Windows
struct L1SchedulerWindow {
  // From the config. file.
  uint32_t start_date ;             // In ticks
  uint32_t end_date ;               // In ticks
  uint32_t partition_id ;           // between 0 and MAX_PARTITION_NUMBER-1
  uint32_t partition_period_start ; // A boolean, as specified in the standard
} ;

// Accesses to this should be well-studied, to ensure
// consistency. I could even make some consistency checks
// each time I re-enter a partition (e.g. check heap 
// and/or stack).
struct L1SchedulerState {
  // Was the L1 scheduler initialized?
  uint32_t scheduler_initialized ;
  // Page table
  struct PageDirectoryEntry* page_table ;
  //----------------------
  // Channel information
  //----------------------
  uint32_t number_of_channels ;
  struct Channel channels[MAX_CHANNELS] ;
  //----------------------
  // Partition information
  //----------------------
  // If yes, number of partitions and active partition.
  // The partition of index 0 is always the system partition.
  uint32_t number_of_partitions ;
  uint32_t active_partition ;
  // Partition information.
  struct L1SchedulerPartitionState partition_state[MAX_PARTITION_NUMBER] ;
  //----------------------------------
  // Real-time information and windows
  //----------------------------------
  // First, information from the config. file and computed from it.
  uint32_t tick_length ; // Tick length, in microseconds
  uint32_t mtf ; // In ticks
  uint32_t number_of_windows ;
  struct L1SchedulerWindow window_list[MAX_WINDOW_NUMBER] ;
  // Now, values that change during scheduling (the actual state).
  uint32_t tick_counter ; // In ticks modulo mtf
  uint32_t active_window ;

  // Number of MTF starts since system start
  uint32_t mtf_start_counter ;
} ;

// Static allocation in the data segment. 
// I could have allocated it at a fixed address,
// but for now it's simpler this way.
extern struct L1SchedulerState l1_scheduler_state ;
//========================================================================

//========================================================================
// Prototypes of the various functions that make up the
// scheduler. We only list the main functions, directly called
// from the interrupt handlers.

// The 
void     L1_scheduler(enum L2_event_type l2_request_type,
		      void* l2_request_data_ptr) ;
void     L1_restore_process_context(struct FullRegisterSet* caller_context,
				    struct FullRegisterSet* target_context) ;
void     L1_reset_partition(PARTITION_ID_TYPE part_id) ;
uint32_t L1_set_partition_mode(OPERATING_MODE_TYPE new_mode) ;
void     L1_handle_partition_error(PARTITION_ID_TYPE partition_id) ;
SYSTEM_TIME_TYPE L1_get_time(void) ;
void     L1_exit_request(void* l2_request_data_ptr) ;

// Debug printing function
void print_full_kernel_config(void) ;
//========================================================================



#endif
