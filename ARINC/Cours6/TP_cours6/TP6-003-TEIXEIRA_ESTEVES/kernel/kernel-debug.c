
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
#include <libc/stdio.h>
#include <librpi/debug.h>
#include <librpi/indent.h>
#include <kernel/kernel-debug.h>
#include <kernel/scheduler.h>

/* Atomic print line function.
 * Printing stops at the first \n or at the NULL 
 * terminator.
 * Its calling context should ensure atomicity for 
 * consistent printing. Thus, it should only be called 
 * by the kernel (not even the system partition). The 
 * partitions should only do this through system call.
 */
void debug_print_line(const char line_prefix,
		      const unsigned char local_indent_depth,
		      const char* str) {
  uart_putc(line_prefix) ;
  uart_putc(' ') ;
  char buf[32] ;
  snprintf(buf,31,"%d:%d",
	   l1_scheduler_state.mtf_start_counter,
	   l1_scheduler_state.tick_counter) ;
  uart_puts(buf) ;
  for(int i=0;i<local_indent_depth;i++) {
    uart_putc('\t') ;
  }
  while (*str) {
    if((*str >= 0x20)&&(*str <= 0x7e)) {
      uart_putc(*str) ;
    } else {
      if(*str == '\n') goto out ;
      if(*str == '\t') uart_putc('\t') ;
      else {
	snprintf(buf,31,"[CTRL:%d]",*str) ;
	uart_puts(buf) ;
      }
    }
    str ++ ;
  }
 out:
  uart_puts("\n") ;
}

// General printline routine called from within the kernel.
void debug_print_line_atomic(const char* str) {
  debug_print_line('K',indent_get_depth(),str) ;
}

// Special printline routine used only for the system
// partition.
void debug_print_line_system_partition(const char* str) {
  debug_print_line('S',1,str) ;
}

// Special printline routine used only for the user
// partitions. Its inputs allow more precise
// printing, including the partition id, whether
// the code is L2 kernel or task, and the task id.
// This routine is the only place in the L1 kernel
// where I explicitly manipulate process identifiers.
void debug_print_line_partition(uint32_t partition_id,
				const char* str,
				int32_t process_id) {
  char buf[256] ;
  struct L1SchedulerPartitionState* partition_state =
    &l1_scheduler_state.
    partition_state[l1_scheduler_state.active_partition] ;
  if(partition_state->in_a_process) {
    snprintf(buf,255,
	     "T%d %s\n",
	     process_id,
	     str) ;
  } else {
    snprintf(buf,255,
	     "L2 %s\n",
	     str) ;
  }
  debug_print_line('0'+partition_id,1,buf) ;
}
