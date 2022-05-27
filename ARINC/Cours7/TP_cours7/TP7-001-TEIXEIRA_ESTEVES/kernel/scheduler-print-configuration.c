
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
#include <librpi/debug.h>        // debug_printf
#include <kernel/scheduler.h>    // For the data structure to print.
#include <librpi/mmu.h>          // Print page table function
#include <librpi/indent.h>


const char* get_partition_mode_name(OPERATING_MODE_TYPE type) {
  switch(type) {
  case IDLE: return "IDLE" ;
  case COLD_START: return "COLD_START" ;
  case WARM_START: return "WARM_START" ;
  case NORMAL: return "NORMAL" ;
  default: return "**ERROR**" ;
  }
}

const char* get_start_condition_name(START_CONDITION_TYPE type) {
  switch(type) {
  case NORMAL_START: return "NORMAL_START" ;
  case PARTITION_RESTART: return "PARTITION_RESTART" ;
  case HM_MODULE_RESTART: return "HM_MODULE_RESTART" ;
  case HM_PARTITION_RESTART: return "HM_PARTITION_RESTART" ;
  default: return "**ERROR**" ;
  }
}

void print_full_kernel_config() {
  int i ;
  debug_printf("scheduler_initialized: %d\n"
	       "page_table: %x\n",
	       l1_scheduler_state.scheduler_initialized,
	       l1_scheduler_state.page_table) ;
  {
    indent_increment_depth() ;
    PrintPageTable(l1_scheduler_state.page_table) ;
    indent_decrement_depth() ;
  }
  debug_printf("number_of_partitions: %d\n"
	       "active_partition: %d\n",
	       l1_scheduler_state.number_of_partitions,
	       l1_scheduler_state.active_partition
	       ) ;
  for(i=0; i< l1_scheduler_state.number_of_partitions; i++) {
    debug_printf("partition %d:\n",i) ;
    { 
      indent_increment_depth() ;
      debug_printf("l2_scheduler_context:\n") ;
      {
	indent_increment_depth() ;
	PrintRegisterSet(&l1_scheduler_state.partition_state[i].
			 l2_scheduler_context) ;
	indent_decrement_depth() ;
      }
      debug_printf("mmap:\n") ;
      {
	indent_increment_depth() ;
	debug_printf("memory_base: %x\n"
		     "memory_size: %x\n"
		     "bss_base:    %x\n"
		     "bss_size:    %x\n"
		     "stack_base:  %x\n"
		     "entry_point: %x\n"
		     "interface:   %x\n",
		     l1_scheduler_state.partition_state[i].mmap.memory_base,
		     l1_scheduler_state.partition_state[i].mmap.memory_size,
		     l1_scheduler_state.partition_state[i].mmap.bss_base,
		     l1_scheduler_state.partition_state[i].mmap.bss_size,
		     l1_scheduler_state.partition_state[i].mmap.stack_base,
		     l1_scheduler_state.partition_state[i].mmap.entry_point,
		     l1_scheduler_state.partition_state[i].mmap.interface) ;
	indent_decrement_depth() ;
      }
      debug_printf("number_of_ports: %u\n",
		   l1_scheduler_state.partition_state[i].number_of_ports) ;
      {
	int j ;
	indent_increment_depth() ;
	for(j=0;j<l1_scheduler_state.partition_state[i].number_of_ports;j++) {
	  debug_printf("Port %d: name:%s direction:%s\n",
		       j,
		       l1_scheduler_state.partition_state[i].ports[j].name,
		       ((l1_scheduler_state.partition_state[i].ports[j].direction==OUT)?"OUT":"IN")
		       ) ;
	}
	indent_decrement_depth() ;
      }
      debug_printf("init_partition_status:\n") ;
      {
	indent_increment_depth() ;
	int32_t tmp ; // Needed to do conversions.
	tmp =
	  l1_scheduler_state.partition_state[i].
	  init_partition_status.PERIOD ;
	debug_printf("PERIOD: %d\n",tmp) ;
	tmp =
	  l1_scheduler_state.partition_state[i].
	  init_partition_status.DURATION ;
	debug_printf("DURATION: %d\n",tmp) ;
	tmp =
	  l1_scheduler_state.partition_state[i].
	  init_partition_status.IDENTIFIER ;
	debug_printf("IDENTIFIER: %d\n",tmp) ;
	tmp =
	  l1_scheduler_state.partition_state[i].
	  init_partition_status.LOCK_LEVEL ;
	debug_printf("LOCK_LEVEL: %d\n",tmp) ;
	debug_printf("OPERATING_MODE: %s\n",
		     get_partition_mode_name(l1_scheduler_state.
					     partition_state[i].
					     init_partition_status.
					     OPERATING_MODE)) ;
	debug_printf("START_CONDITION: %s\n",
		     get_start_condition_name(l1_scheduler_state.
					      partition_state[i].
					      init_partition_status.
					      START_CONDITION)) ;
	indent_decrement_depth() ;
      }
      debug_printf("dacr: 0x%8x\n"
		   "in_a_process: %d\n"
		   "system_ticks_since_last_partition_tick: %d\n",
		   l1_scheduler_state.partition_state[i].dacr.bitvector,
		   l1_scheduler_state.partition_state[i].in_a_process,
		   l1_scheduler_state.partition_state[i].
		   system_ticks_since_last_partition_tick) ;      
      indent_decrement_depth() ;
    }
  }
  debug_printf("number_of_channels: %d\n",
	       l1_scheduler_state.number_of_channels) ;
  for(i=0;i<l1_scheduler_state.number_of_channels;i++) {
    indent_increment_depth() ;
    debug_printf("Channel %d: source(%u,%u) dest(%u,%u) max_msg_size:%u buf:%x first:%u last:%u\n",
		 i,
		 l1_scheduler_state.channels[i].source_partition_id,
		 l1_scheduler_state.channels[i].source_port_id,
		 l1_scheduler_state.channels[i].dest_partition_id,
		 l1_scheduler_state.channels[i].dest_port_id,
		 l1_scheduler_state.channels[i].max_msg_size,
		 l1_scheduler_state.channels[i].data_buffer,
		 l1_scheduler_state.channels[i].first,
		 l1_scheduler_state.channels[i].last
		 ) ;
    indent_decrement_depth() ;
  }
  debug_printf("tick_length: 0x%8x\n"
	       "mtf:         0x%8x\n"
	       "number_of_windows: %d\n",
	       l1_scheduler_state.tick_length,
	       l1_scheduler_state.mtf,
	       l1_scheduler_state.number_of_windows) ;
  for(i=0;i<l1_scheduler_state.number_of_windows;i++) {
    debug_printf("window %d:\n",i) ;
    {
      indent_increment_depth() ;
      debug_printf("start_date:             %d\n"
		   "end_date:               %d\n"
		   "partition_id:           %d\n"
		   "partition_period_start: %d\n",
		   l1_scheduler_state.window_list[i].start_date,
		   l1_scheduler_state.window_list[i].end_date,
		   l1_scheduler_state.window_list[i].partition_id,
		   l1_scheduler_state.window_list[i].partition_period_start) ;
      indent_decrement_depth() ;
    }
  }
  debug_printf("tick_counter:      %d\n"
	       "active_window:     %d\n"
	       "MTF start counter: %d\n",
	       l1_scheduler_state.tick_counter,
	       l1_scheduler_state.active_window,
	       l1_scheduler_state.mtf_start_counter) ;
}

