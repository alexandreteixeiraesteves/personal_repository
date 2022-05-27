
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
#include <librpi/debug.h>
#include <libc/stdlib.h>   // For fatal_error
#include <kernel/init.h>


//------------------------------------------------------------------
// MMU initialization, scheduler-dependent.
//------------------------------------------------------------------
struct PageDirectoryEntry the_page_table[PAGE_TABLE_SIZE]
__attribute__((aligned(PAGE_TABLE_ALIGN)));

// Fill in the page tables in order to allow the MMU to work.
// This function fills **one** page table with default values
// correspondingto the memory model we chose.
// For all partitions (including system), I only set up the
// kernel and peripheral tables. 
void reset_page_table(struct PageDirectoryEntry* current_table) {
  // Counter
  int j ;
  // Set up a nice 1-to-1 mapping between physical and virtual
  // memory, with all pages AP_NOACCESS and DOMAIN_DEFAULT (D15).
  for(j=0;j<PAGE_TABLE_SIZE;j++) {
    current_table[j].physical_addr = j ;
    // By default, no access.
    current_table[j].AP = AP_NOACCESS ;
    // Default domain with no permissions.
    // This corresponds to 0, so I could just remove it.
    current_table[j].Domain = DOMAIN_DEFAULT ;
  }
  // Set up caching for the whole RAM (not for the peripherals).
  for(j=0;j< (GLOBAL_RAM_SIZE>>20);j++) {
    current_table[j].C = 1 ; // Cached
    current_table[j].B = 1 ; // Write-back
  }
  // Set up accessible ranges
  for(j = (KERNEL_CODE_BASE>>20) ; j < (KERNEL_END>>20) ; j++) {
    current_table[j].Type = PDESection ; 
    current_table[j].Domain = DOMAIN_KERNEL ;
    // Only allow access from code running in privileged
    // modes.
    current_table[j].AP = AP_USER_NOACCESS ;
  }
  for(j = (PERIPHERALS_BASE>>20) ; j < (PERIPHERALS_END>>20) ; j++) {
    current_table[j].Type = PDESection ; 
    current_table[j].Domain = DOMAIN_PERIPHERALS ;
    // To allow easy debug, I should leave these pages in
    // full access (in privileged and user modes). This
    // corresponds to the commented line below. But now,
    // I don't really need this, so access from user mode
    // is through system calls only.
    //    current_table[j].AP = AP_READWRITE ;
    current_table[j].AP = AP_USER_NOACCESS ;
    // No code can be executed from here.
    current_table[j].XN = 1 ;
  }
  // Make sure the page table is in the RAM (not in the
  // cache).
  dcache_flush_memory_range((char*)current_table,
			    PAGE_TABLE_SIZE*sizeof(struct PageDirectoryEntry)) ;
}

// The permissions set by this function are as follows:
// - Everybody (user partitions, system partition, kernel) can
//   access the peripherals R/W. Jumps to this area will
//   generate an error (XN bit set).
// - The kernel code has access to the active partition (whose
//   DACR is set). To access other partitions it has to change
//   the DACR.
// - By extension, the system partition code can only access
//   the kernel memory area. To access other partitions'memory
//   area, it has to change the DACR to the one of this partition.
// - User partitions can only access their own memory area
//   (and the peripherals).
// This is obtained by setting:
// - DACR including for each partition (including the system one)
//   to have DOMAIN_KERNEL, DOMAIN_PERIPHERALS, and the
//   partition-specific domain, all with CLIENT permissions.
// - The AP and APX bits of the page records to have
//   * full access, for the user partition pages
//   * full privileged access, no user access for the kernel
//     pages
//   * full access without execution rights, for the peripherals.
// Note that, if I ever try to execute the system partition in
// user mode, I will have to radically change this scheme.
union DomainAccessControlRegister get_partition_dacr(int pid) {
  union DomainAccessControlRegister dacr ;
  dacr.bitvector = 0 ;
  // System partition cannot access the user
  // partitions' memory, because the access bits in the
  // DACR are 0 for the user partitions' domains.
  dacr = SetDomainPermissionBits(dacr,
				 DOMAIN_KERNEL,
				 DA_CLIENT) ; 
  dacr = SetDomainPermissionBits(dacr,
				 DOMAIN_PERIPHERALS,
				 DA_CLIENT) ; 
  if(pid != 0) {
    // User partition (not kernel, not system partition).
    // Non-system partition.
    // For all kernel, and peripheral access,
    // I set permissions to DA_CLIENT, so that permissions
    // are always checked on the pages themselves, not at
    // domain level.
    dacr = SetDomainPermissionBits(dacr,
				   pid,
				   DA_CLIENT) ;
  }
  return dacr ;
}
union DomainAccessControlRegister get_system_dacr() {
  return get_partition_dacr(0) ;
}


// Initializing scheduler data structures, and start the MMU.
// Assumptions:
// - The following constants must be aligned on 1Mbyte
//   pages (i.e. multiples of 0x100000):
//    * KERNEL_CODE_BASE
//    * KERNEL_END
//    * PERIPHERALS_BASE
//    * PERIPHERALS_END
void setup_default_page_tables_and_start_MMU() {
  // Check that the partition table is aligned.
  // Given that I relocate, it's useful to check it.
  if((((uint32_t)the_page_table)>>14)<<14 != (uint32_t)the_page_table) {
    fatal_error("setup_default_page_tables_and_start_MMU: "
		"unaligned page table\n") ;
  }

  // Set a default page table, which only reserves space for the
  // kernel and peripherals.
  reset_page_table(the_page_table) ;
  
  // Finally, enable the MMU and the caches.
  enable_mmu(the_page_table,
	     get_system_dacr()) ;
  enable_data_cache() ;
  enable_instruction_cache() ;
}
