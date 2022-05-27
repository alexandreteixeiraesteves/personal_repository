
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
#ifndef MMU_H
#define MMU_H

// A 16K page directory containing a one-to-one virtual/physical
// mapping can be created with the following (untested) code:
// cf. http://stackoverflow.com/questions/3439708/how-to-enable-arm1136jfs-arm-v6-mmu-to-have-one-to-one-mapping-between-physica
//
// Example here:
//   https://github.com/tzuCarlos/RaspberryPi/tree/master/labMMU
// More info here:
//   http://www.raspberrypi.org/forums/viewtopic.php?f=72&t=91927
//   https://lbd.hackpad.com/MMU-initialization-Jl8YoJ3iUdf
//   https://www.google.fr/search?q=raspberry+pi+mmu+initialization&ie=utf-8&oe=utf-8&gws_rd=cr&ei=LHyqVMalLZftaPSSgMAF

#include <libc/stdint.h>
#include <librpi/mmap.h>

//---------------------------------------------------------
// DomainAccessPermissions
//---------------------------------------------------------

// There are 16 domains, represented on 4 bits.
enum Domain{
  D00 =  0,
  D01 =  1,
  D02 =  2,
  D03 =  3,
  D04 =  4,
  D05 =  5,
  D06 =  6,
  D07 =  7,
  D08 =  8,
  D09 =  9,
  D10 = 10,
  D11 = 11,
  D12 = 12,
  D13 = 13,
  D14 = 14,
  D15 = 15
};

// Each domain gets permissions represented on 
// 2 bits. For each domain the permissions are one 
// the following.
enum DomainAccessPermissions{
  DA_NOACCESS = 0x0,
  DA_CLIENT   = 0x1,
  // Value 0x2 is reserved
  DA_MANAGER  = 0x3
} ;

// Permissions are set on the DomainAccessControl
// registers, as follows.
union DomainAccessControlRegister {
  uint32_t bitvector ;
  // This decoder is, unfortunately, more for
  // helping understanding. Ideally, I should be 
  // able to tabulate here fields of 2 bits wide.
  // However, this is not possible.
  struct {
    uint32_t D00:2 ;
    uint32_t D01:2 ;
    uint32_t D02:2 ;
    uint32_t D03:2 ;
    uint32_t D04:2 ;
    uint32_t D05:2 ;
    uint32_t D06:2 ;
    uint32_t D07:2 ;
    uint32_t D08:2 ;
    uint32_t D09:2 ;
    uint32_t D10:2 ;
    uint32_t D11:2 ;
    uint32_t D12:2 ;
    uint32_t D13:2 ;
    uint32_t D14:2 ;
    uint32_t D15:2 ;
  } decoder ;
} ;

__attribute__((always_inline))
inline union DomainAccessControlRegister
SetDomainPermissionBits(union DomainAccessControlRegister base,
			enum Domain domain,
			enum DomainAccessPermissions perm) {
  // Set the bits to 0
  base.bitvector = base.bitvector & ~(3<<(2*domain)) ;
  // Change them to the new bits
  base.bitvector = base.bitvector | (perm<<(2*domain)) ;
  return base ;
}


//---------------------------------------------------------
// MMU activation and modification definitions.
//---------------------------------------------------------

//---------------------------------------------------------
// Level 1 TLB: entries and manipulation of the 
// table.
//---------------------------------------------------------
//
// The following struct is defined according to 
//   http://infocenter.arm.com/help/index.jsp?topic=/
//      com.arm.doc.ddi0333h/ch06s11s01.html
// Attention, this page describes the default behavior
// of the Arm11, which is backwards-compatible with
// armv4 and armv5 MMU architectures.
// But the RPi allows the use of a second mode, for 
// which the use and the TLB entry structures are
// described here:
//   http://infocenter.arm.com/help/index.jsp?topic=/
//      com.arm.doc.ddi0333h/ch06s11s02.html
// This info was very hard to find.
//
// My use of the TLB is quite rudimentary. I only use
// one page table pointer (TTB0), and not the second.
// How both can be used is explained here: 
//    http://infocenter.arm.com/help/index.jsp?topic=/
//           com.arm.doc.ddi0301h/I1029222.html

// Main selector for page type.
enum PDEType{
  PDETranslationFault = 0,
  PDECoarsePage = 1,
  PDESection = 2
  // Value 3 is reserved
};

// Access to the page.
enum PDE_AP{
  AP_NOACCESS      = 0,  // For everybody 
  AP_USER_NOACCESS = 1,  // R/W for privileged code
                         // no access for user code
  AP_USER_READONLY = 2,  // R/W for privileged code
                         // readonly for users.
  AP_READWRITE     = 3   // For everybody
};

// Each page is associated a page directory entry of 
// 32 bits (one word).
struct PageDirectoryEntry {
  // Used to demux the various forms of TLB entries.
  // For 1Mbyte pages it must be 0x2. To generate a
  // translation fault for all accesses, set it to 0.
  enum PDEType Type:2 ; 
  // Write-back bit (0=write-through, 1=write-back).
  // It's not clear whether write-through works on the
  // RPi.
  uint32_t B:1 ;
  // Cached bit.
  uint32_t C:1 ;
  // Should be zero (cf. doc)
  // Execute-Never bit. It determines if the region 
  // is Executable (0) or Not-executable (1). 
  // For armv4/v5 compatibility, it must be 0.
  uint32_t XN:1 ;
  // The domain of the page (used for defining access
  // configuration, cf. doc).
  uint32_t Domain:4 ;
  // Must be 0 (P is set to indicate that the memory
  // supports ECC, but ARM1176JZ-S processors do not 
  // support the P bit).
  uint32_t P:1 ;
  // First two access permission bits. To set them,
  // take a look at:
  //   http://infocenter.arm.com/help/index.jsp?topic=/
  //     com.arm.doc.ddi0333h/Babeidej.html
  enum PDE_AP AP:2 ;
  // Type Extension Field, used to describe all 
  // the options for inner and outer cachability.
  // Cf. http://infocenter.arm.com/help/index.jsp?topic=/
  //      com.arm.doc.ddi0333h/Babifihd.html
  // bits TEX[1..2] can be redefined to be used for
  // other purposes.
  // To start, I will set TEX to 0.
  uint32_t TEX:3 ;
  // Third access permission bit, but for armv4/v5
  // compatibility it must be 0.
  uint32_t APX:1 ;
  // Shared (S) bit, set to 0 for a non-shared RAM and
  // to 1 for a shared one. Device memory can be Shared 
  // or Non-Shared as determined by the TEX bits and the
  // C and B bits. For armv4/v5 compatibility it must be 0.
  uint32_t S:1 ;
  // Not-Global (nG) bit which determines if the 
  // translation is marked as global (0), or 
  // process-specific (1) in the TLB. For 
  // process-specific translations the translation 
  // is inserted into the TLB using the current ASID, 
  // from the ContextID Register, CP15 c13.
  // To start the work, I set it always to 0.
  uint32_t nG:1 ;
  // Which type of section do I point to? If this bit is
  // set, then it's a 16M super-section. If not (our case)
  // it's a smaller 1Mbyte section. So, for our work it
  // must be 0.
  uint32_t Is16MSection:1 ;
  // Non-secure attribute.
  // Described in:
  //   http://infocenter.arm.com/help/index.jsp?topic=/
  //     com.arm.doc.ddi0333h/Cihebhba.html
  uint32_t NS:1 ;
  // These are the 12 most significant bits of the
  // physical address to which the virtual address is
  // mapped. 12 bits are enough, because we work on
  // 1Mbyte pages.
  uint32_t physical_addr:12 ;
} ;

/*-------------------------------------------------------------*/
/* As per librpi/mmap.h, the page size I will use in the MMU   */
/* is 1MByte, so that a table of 4096 entries                  */
/* covers the 32-bit address space (of 4GByte).                */
/* For some reason, alignment must be on 16384 bytes, which is */
/* exactly the size of the table for 4096 entries.             */
/*-------------------------------------------------------------*/
#define PAGE_TABLE_SIZE    (1<<12)
#define PAGE_TABLE_ALIGN   (PAGE_TABLE_SIZE*sizeof(struct PageDirectoryEntry))

__attribute__((always_inline))
inline uint32_t align_on_page(uint32_t addr) {
  return 
    (((addr>>20)<<20)==addr?addr:((addr>>20)+1)<<20) ;
}

//
void PrintPageTable(struct PageDirectoryEntry* page_table) ;

//
void enable_mmu(struct PageDirectoryEntry* page_table,
		union DomainAccessControlRegister dacr) ;
//
void change_page_table(struct PageDirectoryEntry* new_page_table,
		       union DomainAccessControlRegister dacr) ;


//---------------------------------------------------------
// Cache enable/disable routines.
//---------------------------------------------------------
// 
void enable_data_cache(void) ;
//
void disable_data_cache(void) ;
//
void enable_instruction_cache(void) ;
//
void disable_instruction_cache(void) ;

//---------------------------------------------------------
// Cache flush routines.
//---------------------------------------------------------
// 
void clean_invalidate_data_cache(void) ;
//
void dcache_flush_memory_range(char*start,int size) ;
//
void dcache_inval_memory_range(char*start,int size) ;


#endif
