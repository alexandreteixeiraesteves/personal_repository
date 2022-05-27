
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
#include <libc/stdlib.h>          // For malloc_init, malloc...
#include <libc/string.h>          // For bzero
#include <librpi/uart.h>          // For uart_init, uart_puts
#include <librpi/debug.h>         // For debug_printf
#include <librpi/mmap_c.h>        // For segment pointers (_text_start, etc.)
#include <librpi/mbox.h>          // For reading clock speed
#include <kernel/scheduler.h>
#include <kernel/system-partition.h>
#include <kernel/init.h>

//------------------------------------------------------------------
// Low-level init, scheduler-independent.
// Only covers memory aspects of the kernel.
//------------------------------------------------------------------
void low_level_init() {
  // First, init the UART to allow the printing of messages.
  //uart_init();
  uart_puts("\n");
  uart_puts("\n");
  uart_puts("==============================================\n");
  uart_puts("low_level_init: Init started, UART console on.\n");

  // Init the heap, which means the most things should work.
  init_malloc((char*)KERNEL_HEAP_BASE,KERNEL_HEAP_SIZE) ;
  uart_puts("low_level_init:init_malloc completed.\n");

  // Print the parameters and the segments
  init_debug_printf("low_level_init: stack: %x\n",
		    (unsigned int)get_stack_pointer()) ;
  init_debug_printf("low_level_init: _text start: %x end:%x\n",
		    (uint32_t)&_text_start,(uint32_t)&_text_end) ;
  init_debug_printf("low_level_init: _rodata start: %x end:%x\n",
		    (uint32_t)&_rodata_start,(uint32_t)&_rodata_end) ;
  init_debug_printf("low_level_init: _data start: %x end:%x\n",
		    (uint32_t)&_data_start,(uint32_t)&_data_end) ;
  init_debug_printf("low_level_init: _bss start: %x end:%x\n",
		    (uint32_t)&_bss_start,(uint32_t)&_bss_end) ;
  init_debug_printf("low_level_init: ARM clock (Hz): %d\n",
		    MBoxGetClockRate(MBOX0_PROP_CLK_ARM)) ;
}

void partition_mem_config(struct PartitionConfigData* pcd) {
  struct L1SchedulerPartitionState tmp ;
  memcpy(&tmp,&l1_scheduler_state.partition_state[pcd->id],
	 sizeof(struct L1SchedulerPartitionState)) ;
  // Using the tagged struct initialization of C11
  // The fields not present in the description are set by
  // default to 0, meaning that no bzero is needed.
  l1_scheduler_state.partition_state[pcd->id] =
    (struct L1SchedulerPartitionState)
    {
      .mmap = {
	.memory_base = pcd->memory_base ,
	.memory_size = pcd->memory_size ,
	.bss_base = pcd->bss_base ,
	.bss_size = pcd->bss_size ,
	.stack_base = pcd->stack_base ,
	.entry_point = pcd->entry_point ,
	.interface = pcd->interface
      },
      .init_partition_status = {
	.PERIOD = pcd->period ,
	.DURATION        = 0 ,
	.IDENTIFIER      = pcd->id ,
	.LOCK_LEVEL      = 0 ,
	// It's WARM_START, not COLD_START, because the code is
	// already loaded and primed for execution (bss zeroed),
	// cf. Arinc Specification 653, Part 1, page 18,
	// section 2.3.1.4.1.
	.OPERATING_MODE  = WARM_START ,
	.START_CONDITION = NORMAL_START
      },
      .dacr = pcd->dacr
    } ;
  // The following two are merely copied back, so that they
  // are not erased by the default behavior of this construct,
  // which is to zero every non-affected field.
  l1_scheduler_state.partition_state[pcd->id].number_of_ports =
    tmp.number_of_ports ;
  memcpy(&l1_scheduler_state.partition_state[pcd->id].ports[0],
	 &tmp.ports[0],
	 MAX_PARTITION_PORTS*sizeof(struct PartitionPort)) ;
  // in_a_process is set to 0, so it's OK.
  // Set the interface based on the master copy.
  L1_reset_partition(pcd->id) ;
}
