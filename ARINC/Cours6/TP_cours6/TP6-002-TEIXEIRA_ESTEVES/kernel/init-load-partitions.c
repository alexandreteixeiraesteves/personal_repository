
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
//---------------------------------------------------------------------------
#include <libc/stdlib.h>          // For malloc_init, malloc, fatal_error...
#include <libc/stddef.h>          // For UNUSED, NULL...
#include <libc/string.h>          // For bzero
#include <libc/stdio.h>           // For sprintf, sscanf, etc.
#include <librpi/debug.h>         // For debug_printf
#include <librpi/mmu.h>           // For MMU init, page table...
#include <librpi/uart.h>          // For uart_init, uart_puts
#include <librpi/mmap_c.h>        // For segment pointers (_text_start, etc.)
#include <librpi/elf.h>           // For ELF loading routines.
#include <librpi/system_timer.h>  // For timer configuration.
#include <librpi/mbox.h>          // For reading clock speed
#include <libsdfs/fs.h>           // For SD card access and FAT filesystem
//---------------------------------------------------------------------------
#include <kernel/scheduler.h>        // General scheduler data structures,
                                     // which are initialized here.
#include <kernel/system-partition.h> // Data structures and entry point for
                                     // the system partition.
#include <kernel/init.h>



//------------------------------------------------------------------
// Aux function used in this file only (reading one file).
//------------------------------------------------------------------
// Load one file from the SD card, assuming libfs_init
// has been already called.
char* load_file(const char* file_name) {
  FILE* f = fopen(file_name, "r") ;        
  if(!f) {
    init_debug_printf("load_file: cannot open file %s\n",
		      file_name) ;
    fatal_error("load_file: aborting execution.") ;
  } 
  uint32_t flen = fsize(f);
  char* file_buf = (char *)malloc(flen+1);
  fread(file_buf, 1, flen, f);
  fclose(f);
  file_buf[flen] = 0; // null terminate, for safety.
  init_debug_printf("load_file: complete.\n") ;
  return file_buf ;
}


//------------------------------------------------------------------
// Read the ports of a partition (system or user, syntax and data
// structures are the same. The function also repositions the
// reading pointer in the read string.
//------------------------------------------------------------------
char* read_partition_ports(char* tmp_config_file_buf,
			   uint32_t partition_id) {
  uint32_t i ;
  struct L1SchedulerPartitionState* current_partition = 
    &l1_scheduler_state.partition_state[partition_id] ;
  skip_line(tmp_config_file_buf) ;
  if(sscanf(tmp_config_file_buf,"ports %u",
	    &current_partition->number_of_ports)!=1) {
    fatal_error("read_partition_ports: Cannot load number of ports.\n") ;
  }
  init_debug_printf("read_partition_ports got %d ports.\n",current_partition->number_of_ports) ;
  for(i=0;i<current_partition->number_of_ports;i++) {
    struct PartitionPort* curr_port =
      &current_partition->ports[i] ;
    uint32_t port_id ; // Unused.
    uint32_t port_direction ; 
    skip_line(tmp_config_file_buf) ;
    if(sscanf(tmp_config_file_buf,
	      "%u %s %u",
	      &port_id,
	      curr_port->name,
	      &port_direction) != 3) {
      fatal_error("read_partition_ports: Cannot load port.\n") ;
    }
    curr_port->direction =(port_direction?OUT:IN) ;
  }
  return tmp_config_file_buf ;
}



//------------------------------------------------------------------
// Load the configuration from the SD card and the partitions. 
// This requires the initialization of the SD card, reading 
// the config file, and then reading the ELF files of the 
// partitions.
//------------------------------------------------------------------
void load_partitions() {
  uint32_t i,j ;
  // Temporary storage for configuration data. I prefer setting it
  // once at the end instead of a progressive approach. This way,
  // it's easier to see if there are missing initializations.
  struct PartitionConfigData pcd[MAX_PARTITION_NUMBER] ;
  
  // Populate the L1 scheduler state data structure with default
  // values corresponding to the case where no partition exists
  // and where the partition tables are empty. The only element
  // different from 0 is the address of the page table, which is
  // set up here.
  // The bzero part is normally not needed, because
  // l1_scheduler_state has been zeroed as part of .bss.
  bzero((char*)&l1_scheduler_state,sizeof(struct L1SchedulerState)) ;
  
  //----------------------------------------------------------------------------
  // I start by setting up the "initialized" flag and the page table.
  // It's OK to do it here (and easier to read the code).
  l1_scheduler_state.scheduler_initialized = 1 ;
  l1_scheduler_state.page_table = the_page_table ;
  
  //----------------------------------------------------------------------------
  // Load configuration file, after initializing SD card access.
  libfs_init();
  char* config_file_buf = load_file(CONFIG_FILE_NAME) ;
  char* tmp_config_file_buf = config_file_buf ;
  init_debug_printf("load_partitions: config file content:\n%s",config_file_buf) ;
  // Parse configuration file and incrementally load the partitions
  // and populate the scheduler state.

  //----------------------------------------------------------------------------
  // Read and set the tick length (the period of the timer, in microseconds).
  if(sscanf(tmp_config_file_buf,"tick %x",&l1_scheduler_state.tick_length)!=1) {
    fatal_error("load_partitions: Cannot load tick length.") ;
  }
  if(l1_scheduler_state.tick_length == 0) {
    fatal_error("load_partitions: Tick length must be strictly positive.") ;
  }
  init_debug_printf("load_partitions: tick: %x\n",l1_scheduler_state.tick_length) ;

  //----------------------------------------------------------------------------
  // Read and set the MTF
  skip_line(tmp_config_file_buf) ;
  if(sscanf(tmp_config_file_buf,"mtf %x",&l1_scheduler_state.mtf)!=1) {
    fatal_error("load_partitions: Cannot load mtf size.") ;
  }
  if(l1_scheduler_state.mtf%l1_scheduler_state.tick_length !=0) {
    fatal_error("load_partitions: mtf must be a multiple of tick_length.") ;
  }
  // Convert in number of ticks. It's easier to do scheduling in the
  // tick time base modulo MTF.
  l1_scheduler_state.mtf = 
    l1_scheduler_state.mtf/l1_scheduler_state.tick_length ;
  
  //----------------------------------------------------------------------------
  // Read and set the number of partitions.
  skip_line(tmp_config_file_buf) ;
  skip_line(tmp_config_file_buf) ;
  if(sscanf(tmp_config_file_buf,"partitions %u",
	    &(l1_scheduler_state.number_of_partitions))!=1) {
    fatal_error("load_partitions: Could not read number of partitions.") ;
  }
  // Add the system partition to the total number of partitions.
  init_debug_printf("load_partitions: %d total partitions\n",
		    l1_scheduler_state.number_of_partitions) ;
  if(l1_scheduler_state.number_of_partitions<=1) {
    fatal_error("load_partitions: No user partitions.") ;
  } else if (l1_scheduler_state.number_of_partitions >
	     SYSTEM_LIMIT_NUMBER_OF_PARTITIONS) {
    fatal_error("load_partitions: Number of partitions > "
		"SYSTEM_LIMIT_NUMBER_OF_PARTITIONS.") ;
  }

  //----------------------------------------------------------------------------
  // Read the system partition config and set it in the temporary structure
  {
    uint32_t system_partition_id ;
    uint32_t system_partition_period ;
    skip_line(tmp_config_file_buf) ;
    if(sscanf(tmp_config_file_buf,"%u %x",
	      &system_partition_id,
	      &system_partition_period)!=2) {
      fatal_error("load_partitions: Could not read system partition config.") ;
    }
    init_debug_printf("load_partitions: read system partition config:\t"
		      "\tid: %d period:0x%x\n",
		      system_partition_id,
		      system_partition_period) ;
    if(system_partition_id != 0){
      fatal_error("load_partitions: Bad system partition id.") ;
    }
    if(system_partition_period%l1_scheduler_state.tick_length != 0) {
      fatal_error("load_partitions: system partition period must be a multiple of tick_length.") ;
    }
    system_partition_period = system_partition_period/l1_scheduler_state.tick_length ;
    {
      pcd[system_partition_id].id = 0 ;
      pcd[system_partition_id].memory_base = 0  ;
      pcd[system_partition_id].memory_size = 0  ;
      pcd[system_partition_id].bss_base = 0  ;
      pcd[system_partition_id].bss_size = 0  ;
      pcd[system_partition_id].stack_base = SYSTEM_STACK_BASE ;
      pcd[system_partition_id].entry_point = (uint32_t)system_partition_entry_point ;
      pcd[system_partition_id].dacr = get_partition_dacr(0) ;
      pcd[system_partition_id].interface = &system_partition_interface ;
      pcd[system_partition_id].period = system_partition_period ;
      pcd[system_partition_id].elf_file_name[0] = '\0' ;
    }
    // Read the ports of the system partition
    tmp_config_file_buf = read_partition_ports(tmp_config_file_buf,0) ;
  }

  //----------------------------------------------------------------------------
  // For each user partition:
  // - read the partition configuration from the config. file
  // - load the partition and relocate the code
  // During both steps, fill in the partition-related scheduler data
  //----------------------------------------------------------------------------

  /************************************************************************/
  /* Step 1: Reading partition configuration from the config file for one */
  /*         partition and fill in the temporary data structure.          */
  /*         Fills in id, memory_base, memory_size, elf_file_name,        */
  /*         period, stack_base.                                          */
  /************************************************************************/
  {
    // I start to put partitions after the end of the kernel (aligned on
    // memory page size). This variable is an accumulator used to place
    // partitions one after the next.
    uint32_t partition_base = align_on_page(KERNEL_END) ;
    
    for(i=1;i<l1_scheduler_state.number_of_partitions;i++) {
      init_debug_printf("load_partitions: loading partition %d Step1.\n",i) ;
      
      // Set up the id and the memory base.
      pcd[i].id = i ;
      pcd[i].memory_base = partition_base ;
      
      // Advance in the config file and read the partition record.
      skip_line(tmp_config_file_buf) ;
      {
	uint32_t tmp_id ;
	if(sscanf(tmp_config_file_buf,"%u %s %x %x",
		  &tmp_id,
		  pcd[i].elf_file_name,
		  &pcd[i].memory_size,
		  &pcd[i].period)!=4) {
	  fatal_error("load_partitions: Could not read config data.") ;
	}
	init_debug_printf("load_partitions: read partition %d, id %d, file %s size %x period %x\n",
			  i,
			  tmp_id,
			  pcd[i].elf_file_name,
			  pcd[i].memory_size,
			  pcd[i].period) ;
	if(tmp_id != i) {
	  fatal_error("load_partitions: Incorrect partition ID.") ;
	}
	// Align on page size
	pcd[i].memory_size = align_on_page(pcd[i].memory_size) ;
	// 
	if(pcd[i].period%l1_scheduler_state.tick_length != 0) {
	  fatal_error("load_partitions: partition periods must be multiple of tick_length.") ;
	}
	pcd[i].period = pcd[i].period / l1_scheduler_state.tick_length ;
      }
      // Set the stack at the end of the partition memory area.
      pcd[i].stack_base = pcd[i].memory_base + pcd[i].memory_size ;
      // The next partition will start just after the current one.
      partition_base = pcd[i].stack_base ;
      // Now, read the ports of the partition.
      tmp_config_file_buf = read_partition_ports(tmp_config_file_buf,i) ;
    }
  }

  /************************************************************************/
  /*  Step 2. Set up the page table and user rights.                      */
  /*          Fills in dacr.                                              */
  /************************************************************************/
  for(i=1;i<l1_scheduler_state.number_of_partitions;i++) {
    init_debug_printf("load_partitions: loading partition %d Step 2.\n",i) ;
    // Set up the permissions word (load version).
    pcd[i].dacr = get_partition_dacr(i) ;
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Before configuring the partition memory or
    // loading partition code, I have to make accessible the
    // partition memory space by updating
    // the corresponding page table. This provides access to
    // the pages that will contain code and data.
    init_debug_printf("load_partitions: Update partition page table...\n");
    // Add the partition pages to the page table
    uint32_t first_page = pcd[i].memory_base >> 20 ;
    uint32_t limit_page = (pcd[i].memory_base+pcd[i].memory_size) >> 20 ;
    for(j = first_page ; j < limit_page ; j++) {
      // Cached task code, data, and stack
      l1_scheduler_state.page_table[j].Type = PDESection ; 
      l1_scheduler_state.page_table[j].Domain = i ;
      l1_scheduler_state.page_table[j].AP = AP_READWRITE ;
    }
    // Make sure the data is saved in memory, because
    // the TLB needs to take it from there.
    // I could do it all at once at the end, but why not do it
    // bit by bit.
    dcache_flush_memory_range((char*)(l1_scheduler_state.page_table+first_page),
			      (limit_page-first_page)*
			      sizeof(struct PageDirectoryEntry)) ;
  }


  /************************************************************************/
  /*  Step 3. Load the ELF file (just the load from the SD card).         */
  /************************************************************************/
  for(i=1;i<l1_scheduler_state.number_of_partitions;i++) {
    init_debug_printf("load_partitions: loading partition %d Step 3.\n",i) ;
    // Change the page table (actually, only the permission word) to make
    // the partition memory area accessible.
    change_page_table(l1_scheduler_state.page_table,
		      pcd[i].dacr) ;
    // 
    init_debug_printf("load_partitions: part %u will be loaded with:\n"
		      "\tmemory_base=%8x  memory_size=%8x\n"
		      "\tstack_base =%8x  ELF file =%s\n",
		      i,
		      pcd[i].memory_base,
		      pcd[i].memory_size,
		      pcd[i].stack_base,
		      pcd[i].elf_file_name) ;
    //
    FILE* f = fopen(pcd[i].elf_file_name, "r") ;
    if(f == NULL) {
      fatal_error("load_partitions: Cannot open file.") ;
    }
    init_debug_printf("load_partitions: ELF file opened.\n") ;
    uint32_t elf_size = fsize(f);
    // I require, at a minimum, to have MIN_PARTITION_FREE_RAM for the
    // stack and heap.
    if(elf_size + MIN_PARTITION_FREE_RAM >= pcd[i].memory_size) {
      fatal_error("load_partitions: I should have provisioned more.") ;
    }
    fread((char*)pcd[i].memory_base,1,elf_size,f);
    fclose(f);
  }

  /************************************************************************/
  /*  Step 4. Relocate the ELF file and set up the entry point and bss    */
  /*          data.                                                       */
  /*          Fills in entry_point, bss_start, bss_size, interface.       */
  /************************************************************************/
  for(i=1;i<l1_scheduler_state.number_of_partitions;i++) {
    init_debug_printf("load_partitions: loading partition %d Step 4.\n",i) ;
    // Change the page table (actually, only the permission word) to make
    // the partition memory area accessible.
    change_page_table(l1_scheduler_state.page_table,
		      pcd[i].dacr) ;
    int32_t err = 
      relocate_elf_file((char*)pcd[i].memory_base,
			&pcd[i].entry_point,
			&pcd[i].bss_base,
			&pcd[i].bss_size) ;
    if(err != 0){
      init_debug_printf("load_partitions: ELF relocating error %d\n",err) ;
      fatal_error("load_partitions: Aborting.") ;
    } else {
      init_debug_printf("load_partitions: ELF file relocated with:\n"
			"\tentry_point: %x\n"
			"\tbss_base: %x\n"
			"\tbss_size: %x\n",
			pcd[i].entry_point,
			pcd[i].bss_base,
			pcd[i].bss_size) ;
    }
    // Flush the partition code to memory to ensure I don't have
    // heisenbugs due to modified text that is still in the data cache.
    // I could have flushed only elf_size, but I prefer more radical
    // solutions in this config phase.
    dcache_flush_memory_range((char*)pcd[i].memory_base,
			      pcd[i].memory_size) ;
    // Finally, set up the interface.
    pcd[i].interface = (struct L1PartitionInterface*)pcd[i].bss_base ;
  }


  //----------------------------------------------------------------------------
  // Now, for **all** partitions, set up the kernel data and reset the
  // partition to initial state. This also requires changing the
  // page table.
  //----------------------------------------------------------------------------
  for(i=0;i<l1_scheduler_state.number_of_partitions;i++) {
    init_debug_printf("load_partitions: Partition %d: "
		      "Set up L1/L2 kernel data and reset.\n",i) ;
    // Change the page table (actually, only the permission word) to make
    // the partition memory area accessible.
    change_page_table(l1_scheduler_state.page_table,
		      pcd[i].dacr) ;
    // No need to flush the data cache, because I no longer mixed
    // code and data.
    partition_mem_config(&pcd[i]) ;
  }
  
  //----------------------------------------------------------------------------
  // Load time partitioning from config file.
  // - the list of windows and their properties.
  // During load, fill in the scheduler data structures.
  //
  // Time values are in microseconds, which is the base of the system timer
  // on the Raspberry Pi.
  //----------------------------------------------------------------------------    
  {
    // Load timing information. First, the tick length.
    skip_line(tmp_config_file_buf) ;
    skip_line(tmp_config_file_buf) ;
    if(sscanf(tmp_config_file_buf,"windows %u",&l1_scheduler_state.number_of_windows)!=1) {
      fatal_error("load_partitions: Cannot load number of windows.") ;
    }
    if((l1_scheduler_state.number_of_windows==0)||
       (l1_scheduler_state.number_of_windows>SYSTEM_LIMIT_NUMBER_OF_WINDOWS)) {
      fatal_error("load_partitions: Bad number of windows.") ;
    }
    for(i=0;i<l1_scheduler_state.number_of_windows;i++) {
      skip_line(tmp_config_file_buf) ;
      uint32_t window_number ;
      uint32_t start_date ;
      uint32_t end_date ;
      uint32_t partition_id ;
      uint32_t partition_period_start ;
      if(sscanf(tmp_config_file_buf,"%u %x %x %u %u",
		&window_number,&start_date,
		&end_date,&partition_id,&partition_period_start)!=5) {
	init_debug_printf("load_partitions: window %d:\n",i) ;
	fatal_error("load_partitions: Cannot load window.") ;
      }
      if((start_date%l1_scheduler_state.tick_length!=0)||
	 (end_date%l1_scheduler_state.tick_length!=0)) {
	init_debug_printf("load_partitions: window %d: start: %x end: %x pps: %d tick: %x \n",
			  i,
			  start_date,
			  end_date,
			  partition_period_start,
			  l1_scheduler_state.tick_length) ;
	fatal_error("load_partitions: start_date, end_date must be a multiple of tick_length.") ;
      }
      // Convert in number of ticks. It's easier to do scheduling in the
      // tick time base modulo MTF.
      start_date = start_date/l1_scheduler_state.tick_length ;
      end_date = end_date/l1_scheduler_state.tick_length ;
      if(start_date>=end_date) {
	fatal_error("load_partitions: inconsistent start/end dates.") ;
      }
      if(i!=0){
	if(start_date != l1_scheduler_state.window_list[i-1].end_date) {
	  fatal_error("load_partitions: non-contiguous windows.") ;
	}
      } else {
	if(start_date != 0) {
	  fatal_error("load_partitions: windows do not start at 0.") ;
	}
      }
      if(i==l1_scheduler_state.number_of_windows-1) {
	if(end_date != l1_scheduler_state.mtf) {
	  fatal_error("load_partitions: windows do not fill MTF.") ;
	}
      }
      if(partition_id >= l1_scheduler_state.number_of_partitions) {
	fatal_error("load_partitions: window of unexistent partition.") ;
      }
      l1_scheduler_state.window_list[i].start_date = start_date ;
      l1_scheduler_state.window_list[i].end_date = end_date ;
      l1_scheduler_state.window_list[i].partition_id = partition_id ;
      l1_scheduler_state.window_list[i].partition_period_start =
	partition_period_start ;
    }
    init_debug_printf("load_partitions: tick: %x   mtf: %x  windows: %d\n",
		      l1_scheduler_state.tick_length,
		      l1_scheduler_state.mtf,
		      l1_scheduler_state.number_of_windows) ;
    for(i=0;i<l1_scheduler_state.number_of_windows;i++) {
      init_debug_printf("\t window %d\tstart %8x end %8x partition %d\n",
			i,
			l1_scheduler_state.window_list[i].start_date,
			l1_scheduler_state.window_list[i].end_date,
			l1_scheduler_state.window_list[i].partition_id) ;
    }
  }


  //----------------------------------------------------------------------------
  // Load channels from config file.
  //----------------------------------------------------------------------------    
  {
    skip_line(tmp_config_file_buf) ;
    skip_line(tmp_config_file_buf) ;
    if(sscanf(tmp_config_file_buf,
	      "channels %u",
	      &l1_scheduler_state.number_of_channels)!=1) {
      fatal_error("load_partitions: Cannot load number of channels.") ;
    }
    for(i=0;i<l1_scheduler_state.number_of_channels;i++) {
      skip_line(tmp_config_file_buf) ;
      uint32_t channel_id ;
      if(sscanf(tmp_config_file_buf,"%u %u %u %u %u %u",
		&channel_id,
		&l1_scheduler_state.channels[i].source_partition_id,
		&l1_scheduler_state.channels[i].source_port_id,
		&l1_scheduler_state.channels[i].dest_partition_id,
		&l1_scheduler_state.channels[i].dest_port_id,
		&l1_scheduler_state.channels[i].max_msg_size) != 6) {
	init_debug_printf("load_partitions: channel %d: error loading\n",i) ;
	fatal_error("load_partitions: Cannot load channel.") ;
      }
      int k ;
      for(k=0;k<CHANNEL_BUFFER_SIZE;k++) {
	l1_scheduler_state.channels[i].data_buffer[k].data = 
	  (unsigned char*)malloc(l1_scheduler_state.channels[i].max_msg_size) ;
      }
      l1_scheduler_state.channels[i].first = 0 ;
      l1_scheduler_state.channels[i].last = 0 ;
    }
  }
  

  // Free the buffer allocated for the file data.
  // It is no longer needed.
  free(config_file_buf) ;


  
  // Finally, set the L1 scheduler in its start configuration.  Some
  // of this has been done while loading the partitions (setting them
  // in RPI_NON_INITIALIZED mode, etc.).  But we still must set up the active
  // window, tick counter, and active partition so that the first
  // execution cycle starts window 0.
  {
    l1_scheduler_state.active_window =
      l1_scheduler_state.number_of_windows - 1 ;
    l1_scheduler_state.tick_counter =  l1_scheduler_state.mtf - 1 ;
    l1_scheduler_state.active_partition = 
      l1_scheduler_state.window_list[l1_scheduler_state.active_window].
      partition_id ;
    // Now, I have to enable access to the active partition.
    // Without this, scheduler accesses to this region will be
    // refused.
    change_page_table(l1_scheduler_state.page_table,
		      pcd[l1_scheduler_state.active_partition].dacr) ;
      
    // For completeness, I could also render active the memory of this
    // partition_id. But it's not needed, because I'm in privileged
    // code with full acess.

    // Debug print and return.
    //init_debug_printf("load_partitions: active_window %d  "
    //		      "tick_counter %x  active_partition %d\n",
    //		      l1_scheduler_state.active_window,
    //		      l1_scheduler_state.tick_counter,
    //		      l1_scheduler_state.active_partition) ;
  }
}
