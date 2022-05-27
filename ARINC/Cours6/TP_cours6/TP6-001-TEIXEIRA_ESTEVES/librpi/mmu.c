
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
#include <libc/stddef.h>    // For NULL
#include <libc/stdlib.h>    // For fatal_error
#include <librpi/debug.h>   // For debug_printf
#include <librpi/mmu.h>


//---------------------------------------------------------
// Printing
//---------------------------------------------------------
void PrintPageTable(struct PageDirectoryEntry* page_table) {
  int i ;
  for(i=0;i<PAGE_TABLE_SIZE;i++) {
    if(page_table[i].Type != PDETranslationFault) {
      debug_printf("Entry %8x -> %8x (phys: %x)\n",
		   i,
		   page_table[i],
		   page_table[i].physical_addr) ;
    }
  }
}


//---------------------------------------------------------
// Domain control register access operations.
//---------------------------------------------------------
// Setting the DomainAccessControlRegister can only be done in
// privileged mode. To ensure spatial isolation, this implies
// that level 2 scheduler must work in user mode...
__attribute__((always_inline))
inline void 
SetDomainControlRegister(union DomainAccessControlRegister reg) {
  asm volatile("mcr p15, 0, %0, c3, c0, 0"
	       : : "r" (reg.bitvector) : "memory");
}
// Reading the DomainAccessControlRegister.
__attribute__((always_inline))
inline union DomainAccessControlRegister
GetDomainControlRegister() {
  union DomainAccessControlRegister result ;
  asm volatile("mrc p15, 0, %0, c3, c0, 0"
	       : "=r" (result.bitvector) : : "memory");
  return result ;
}


//---------------------------------------------------------
// MMU activation and modification routines.
//---------------------------------------------------------

/* Enable the MMU when the MMU is not yet enabled. 
   The domain access control register must be different 
   from 0. For instance, setting it to 0xFFFFFFFF, 
   gives all permissions to all domains.
 */
void enable_mmu(struct PageDirectoryEntry* page_table,
		union DomainAccessControlRegister dacr) {
  //  debug_printf("enable_mmu: Entered with page table=%x dacr=%x\n",
  //	       (uint32_t)page_table,
  //	       dacr.bitvector) ;
  // Check if the page table is non-NULL
  if(page_table==NULL) {
    fatal_error("enable_mmu: table pointer is NULL.\n"
		"Exiting...\n") ;
  }
  uint32_t c1 ;
  /* Read cr15, c1. */
  asm volatile("mrc p15, 0, %[x], c1, c0, 0"
	       : [x]"=r"(c1) :: "memory") ;
  if(c1 & 0x1) {
    /* The cache is already enabled. Exit with an error
       message. */
    fatal_error("enable_mmu: cache already enabled.\n"
		"Exiting...\n") ;
  }
  /* Copy the page table address to cp15 */
  asm volatile("mcr p15, 0, %0, c2, c0, 0"
	       : : "r" (page_table) : "memory");
  /* Setup the domain access control register. */
  if(dacr.bitvector == 0) {
    fatal_error("enable_mmu: access control register is empty.\n"
		"Exiting...\n") ;
  }
  SetDomainControlRegister(dacr) ;
  //debug_printf("enable_mmu: before actual enabling.\n");
  /* Enable the MMU by setting bit 0 of c1 to 1.*/
  c1 |= 0x1 ;
  asm volatile("mcr p15, 0, %[x], c1, c0, 0"
	       :: [x]"r"(c1) : "memory" );
}

/* Change the active page table.
   Attention: make sure that the page is written to memory
   before calling this function. If part of it is still 
   in the data cache, then the result may be very nasty 
   heisenbugs. 
   The domain access control register is set only
   if it is different from 0. Otherwise, it is left
   unchanged.
*/
void change_page_table(struct PageDirectoryEntry* new_page_table,
		       union DomainAccessControlRegister dacr) {
  //debug_printf("change_page_table: Entered with page table=%8x dacr=%8x\n",
  //	       (uint32_t)new_page_table,
  //	       dacr.bitvector) ;
  /* Check if the page table is non-NULL. */
  if(new_page_table==NULL) {
    fatal_error("change_page_table: table pointer is NULL.") ;
  }
  /* Copy the page table address to cp15 */
  asm volatile("mcr p15, 0, %0, c2, c0, 0"
	       : : "r" (new_page_table) : "memory");
  /* Setup the domain access control register. */
  if(dacr.bitvector == 0) {
    fatal_error("change_page_table: permissions are empty.") ;
  }
  SetDomainControlRegister(dacr) ;
  /* Invalidate the whole TLB. Here, in place of c5 I could
     use c7, because it's a unified TLB. */
  asm volatile("mov r0, #0;"
	       "mcr p15, 0, r0, c8, c5, 0"
	       : : : "r0","memory");
  /* Invalidate the memory range of the page table.*/
  //dcache_inval_memory_range((char*)old_page_table,
  //			    PAGE_TABLE_ALIGN) ;
}

//---------------------------------------------------------
// Cache flush routines.
//---------------------------------------------------------

// Explanation of what are cache invalidate and clean operations:
//  http://infocenter.arm.com/help/index.jsp?topic=/
//             com.arm.doc.ddi0092b/ch04s04s05.html
// Description of the RPi processor register at work:
//  http://infocenter.arm.com/help/index.jsp?topic=/
//             com.arm.doc.ddi0201d/ch03s03s05.html
//__attribute__((always_inline))
void clean_invalidate_data_cache() {
  // Clean and invalidate entire data cache, cf. 
  // DDI0301H_arm1176jzfs_r0p7_trm.pdf, page 3-74
  asm volatile("mov r0, #0;" 
	       "mcr p15, 0, r0, c7, c14, 0" ) ;
}

/* Force the writing of all the elements in the
   write buffer. */
__attribute__((always_inline))
inline void dcache_flush_write_buffer() {
  asm volatile("mcr p15, 0, r0, c7, c10, 4");
}

// Cf. http://sandsoftwaresound.net/raspberry-pi/arm11-microarchitecture/
#define L1_DCACHE_LINE_SIZE 32

// Force writeback of data for a memory range.
void dcache_flush_memory_range(char*start,int size) {
  // Clean and invalidate data cache line, cf.
  // DDI0301H_arm1176jzfs_r0p7_trm.pdf, around 
  // page 3-74.
  // First, align the pointer on cache line boundary. Given that 
  // a cache line is 32 bytes long, this consists in removing the
  // 5 least significant bits of start. This corresponds to ignoring
  // the lower bits, like it is done in the MVAs defined in the 
  // manual cited above.
  char* p = (char*)((uint32_t)start & 0xffffffe0) ;
  char* end = start+size ;
  while(p<end) {
    asm volatile("mov r0, %[value];" 
		 "mcr p15, 0, r0, c7, c14, 1"
		 :
		 : [value] "r" (p)
		 :) ;
    p += L1_DCACHE_LINE_SIZE ;
  }
  dcache_flush_write_buffer() ;
}

// Force writeback of data for a memory range.
void dcache_inval_memory_range(char*start,int size) {
  // Invalidate data cache line, cf.
  // DDI0301H_arm1176jzfs_r0p7_trm.pdf,
  // First, align the pointer on cache line boundary. Given that 
  // a cache line is 32 bytes long, this consists in removing the
  // 5 least significant bits of start. This corresponds to ignoring
  // the lower bits, like it is done in the MVAs defined in the 
  // manual cited above.
  char* p = (char*)((uint32_t)start & 0xffffffe0) ;
  char* end = start+size ;
  while(p<end) {
    asm volatile("mov r0, %[value];" 
		 "mcr p15, 0, r0, c7, c6, 1"
		 :
		 : [value] "r" (p)
		 :) ;
    p += L1_DCACHE_LINE_SIZE ;
  }
}

__attribute__((always_inline))
inline void invalidate_instruction_cache() {
  // Clean and invalidate entire data cache, cf. 
  // DDI0301H_arm1176jzfs_r0p7_trm.pdf, page 3-74
  asm volatile("mov r0, #0;" 
	       "mcr p15, 0, r0, c7, c5, 0" ) ;
}


//---------------------------------------------------------
// Cache enable/disable routines.
//---------------------------------------------------------

// Enable data caching once the MMU is set up.
void enable_data_cache() {
  asm volatile("mrc p15, 0, r0, c1, c0, 0 ;" // Read c1 into r0
	       "orr r0, r0, #0x4 ;"          // Set bit 2: Dcache
	       "mcr p15, 0, r0, c1, c0, 0"); // Return r0 to c1
}
// Enable instruction caching once the MMU is set up.
void enable_instruction_cache() {
  asm volatile("mrc p15, 0, r0, c1, c0, 0 ;" // Read c1 into r0
	       "orr r0, r0, #0x1000 ;"       // Set bit 12: Icache
	       "mcr p15, 0, r0, c1, c0, 0"); // Return r0 to c1
}
//
void disable_data_cache() {
  clean_invalidate_data_cache() ;
  invalidate_instruction_cache() ;
  asm volatile("mcr p15, 0, r0, c7, c10, 4");// Write down the write buffer
  asm volatile("mrc p15, 0, r0, c1, c0, 0 ;" // Read c1 into r0
	       "bic r0, r0, #0x4 ;"          // Clear bit 2: Dcache
	       "mcr p15, 0, r0, c1, c0, 0"); // Return r0 to c1
}
//
void disable_instruction_cache() {
  asm volatile("mrc p15, 0, r0, c1, c0, 0 ;" // Read c1 into r0
	       "bic r0, r0, #0x1000 ;"       // Clear bit 12: Icache
	       "mcr p15, 0, r0, c1, c0, 0"); // Return r0 to c1
}

