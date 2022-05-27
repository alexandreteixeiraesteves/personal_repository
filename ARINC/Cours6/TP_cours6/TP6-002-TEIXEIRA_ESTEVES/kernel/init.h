
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
#ifndef INIT_H
#define INIT_H


#include <libc/stdint.h>
#include <librpi/mmu.h>
#include <kernel/scheduler.h>

#define CONFIG_FILE_NAME "config.pok"
// Minimal amount of memory to remain free
// after the load of the elf file.
#define MIN_PARTITION_FREE_RAM 0x40000

// The page table
extern struct PageDirectoryEntry the_page_table[PAGE_TABLE_SIZE]
__attribute__((aligned(PAGE_TABLE_ALIGN))) ;

// For the current configuration, I use the
// following domains:
#define DOMAIN_DEFAULT     D15
#define DOMAIN_PERIPHERALS D14
#define DOMAIN_KERNEL      D00
// For the partitions (including KERNEL=PARTITION0), the
// domain id is the number of the partition. This makes for
// the 13 user partitions limit.


// Structure used to store partition configuration
// data during system load. It's redundant, but better than to pass
// nameless parameters to a function.
struct PartitionConfigData {
  // Needed to fill in the internal scheduler config.
  uint32_t id ;
  uint32_t memory_base ;
  uint32_t memory_size ;
  uint32_t bss_base ;
  uint32_t bss_size ;
  uint32_t stack_base ;
  uint32_t entry_point ;
  union DomainAccessControlRegister dacr ;
  struct L1PartitionInterface* interface ;
  uint32_t period ;
  // Needed to load
  char elf_file_name[32] ;
} ;



// Init functions called from init-kernel.c and the other init
// files.
void low_level_init(void) ;
void load_partitions(void) ;
void setup_default_page_tables_and_start_MMU(void) ;
void partition_mem_config(struct PartitionConfigData* pcd) ;

// Single init function that calls all the other.
void kernel_init(void) ;

// Functions that are also called from other places.
union DomainAccessControlRegister get_system_dacr(void) ;
union DomainAccessControlRegister get_partition_dacr(int pid) ;


//#define init_debug_printf(...) debug_printf(__VA_ARGS__)
#define init_debug_printf(...)
  
#endif
