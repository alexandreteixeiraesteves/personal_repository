
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
#include <libc/stdint.h>
#include <libc/stdio.h>
#include <librpi/debug.h>
#include <libc/string.h>
//---------------------------------
#include <librpi/elf-internals.h>
#include <librpi/elf.h>

//#define elf_debug_printf(...) debug_printf(__VA_ARGS__)
#define elf_debug_printf(...)


/* ELF file loader for the Raspberry Pi.
   The loaded ELF files to load must satisfy the 
   following requirements:
   1. Compiled with -fpic (-fPIC may work, too, but it's 
      not tested).
   2. The loader script places:
      * the text segment first, at address 0x0, 
        with the entry point at the beginning of the
        text segment.
      * the bss segment last 
   
   The loader does the following:
   - It copies the file at a previously-specified address
     in the RaspberryPI memory space.
   - Choose a destination address X.
   - It determines the start of the text segment, the start
     of the bss segment, and copies everything between them
     at the destination address of the partition code.
   - Inside the copied code, locate the .got file and then 
     add X to all the addresses of the .got. I don't know
     if I have to do this for .got.plt (I think not, because
     I don't have relocatable libraries).
 */


// Inspired from Wikipedia and
//   http://www.opensource.apple.com/source/gdb/gdb-1515/
//      src/binutils/readelf.c
//   http://www.opensource.apple.com/source/
//   http://www.opensource.apple.com/source/gdb/gdb-1515/
//      src/include/elf/external.h and internal.h
//   http://eli.thegreenplace.net/2011/11/03/
//      position-independent-code-pic-in-shared-libraries/
//   http://www.codeproject.com/Articles/70302/
//      Redirecting-functions-in-shared-ELF-libraries
//   Google: elf pic .got
// The ELF format doc can be found here:
//   http://www.skyfree.org/linux/references/


// Note: In the ELF files produced by gcc, .text seems to be 
// aligned on 0x8000, even if no alignment directive is given.
// 0x8000 is a default page size in some OSs.
// Assuming that .text is the first segment and .bss is the last
// useful segment of the program, I can do the following when 
// loading:
// - load at the place where I want to get my base -0x8000, so 
//   that I dump the ELF header and padding.
// - discard everything after .bss by simply writing over it.
//
// To access .got:
// - find the unique symtab segment header
// - from the sh_link attribute of this header, get the index
//   of the segment header containing the section names.
// - Get the data offset of this segment.
// - Find the section whose name is ".got".
// - Traverse the whole data of .got and change the uint32_t values
//   by adding the offset where I put the file. I assume the base
//   offset in rpi.cmd is 0.
// Take a look here: 
//   http://www.sourceware.org/ml/binutils/2008-06/msg00115.html

// Very simple ELF file loader that makes lots of 
// assumptions on the way it was generated but should work 
// for my Raspberry Pi setting.
//
// What it does:
// - Relocate by modifying the .got entries. Previously, I used to
//   move .text at the start of file_data (which is the start of the
//   partition memory region. But I no longer want to do this
//   destructive procedure, and I rely on returning the correct
//   entry point at the beginning of .text.
// - Return the position of the .text segment, and the position
//   and size of .bss, to allow its zeroing. When there is no
//   .bss (should be impossible in our case, due to the
//   interface) the value of bss_base is equal to text_start, and
//   bss_size is 0.
int relocate_elf_file(char* file_data,
		      uint32_t* p_text_start, /* output uint32_t value */
		      uint32_t* p_bss_base,   /* output uint32_t value */
		      uint32_t* p_bss_size)   /* output uint32_t value */ {
  elf_debug_printf("relocate_elf_file: Entered with addr: %8x \n",
		   (uint32_t)file_data) ;
  int i ;
  /* Assumes that the ELF file has already been loaded at the address
     file_data, and that this address is word-aligned. */
  if(((uint32_t)file_data)%sizeof(uint32_t)) {
    // The load address is not word-aligned. Exiting.
    return -1 ;
  }
  struct Elf32Header* elf32_header = (struct Elf32Header*)file_data ;
  struct Elf32SectionHeader* section_header =
    (struct Elf32SectionHeader*)(file_data+elf32_header->e_shoff) ;
  // Check the consistency of the strtab section choice.
  if((elf32_header->e_shstrndx <= 0)||
     (elf32_header->e_shstrndx > elf32_header->e_shnum)) {
    return -2 ;
  }
  char* section_name_base = 
    file_data+section_header[elf32_header->e_shstrndx].sh_offset ;
  
  // Find the .got and .bss sections
  int text_section_index = -1 ;
  int got_section_index = -1 ;
  int bss_section_index = -1 ;
  {
    for(i=0;i<elf32_header->e_shnum;i++){
      if(strcmp(section_name_base+section_header[i].sh_name,
		".text")==0){
	if(text_section_index!=-1) {
	  // Found 2 .text sections.
	  return -3 ;
	}
	elf_debug_printf("relocate_elf_file: found .text section.\n") ;
	text_section_index = i ;
      }
      if(strcmp(section_name_base+section_header[i].sh_name,
		".got")==0){
	if(got_section_index!=-1) {
	  // Found 2 .got sections.
	  return -4 ;
	}
	elf_debug_printf("relocate_elf_file: found .got section.\n") ;
	got_section_index = i ;
      }
      if(strcmp(section_name_base+section_header[i].sh_name,
		".bss")==0){
	if(bss_section_index!=-1) {
	  // Found 2 .bss sections.
	  return -5 ;
	}
	elf_debug_printf("relocate_elf_file: found .bss section.\n") ;
	bss_section_index = i ;
      }
    }
    // Sanity checks
    if(text_section_index == -1) {
      // Error: no .text segment, so no entry point.
      return -6 ;
    }
    // I also require that .text is the first section, which
    // helps me compute the relocation distance. Attention, the
    // section of index 0 contains nothing (type NULL, by convention).
    if(text_section_index != 1) {
      return -7 ;
    }
    // I also check whether the start of the .text section is
    // the entry point.
    if(section_header[text_section_index].sh_addr != elf32_header->e_entry) {
      return -8 ;
    }
    // Check that there's a single program.
    if(elf32_header->e_phnum != 1) {
      return -6 ;
    }
  }
  elf_debug_printf("relocate_elf_file: found segments:\n"
		   "\ttext section index = %8x\t addr=%8x\n"
		   "\t got section index = %8x\t addr=%8x\n"
		   "\t bss section index = %8x\t addr=%8x\n",
		   text_section_index,
		   (uint32_t)(file_data + 
			      section_header[text_section_index].sh_offset),
		   got_section_index,
		   (uint32_t)(file_data + 
			      section_header[got_section_index].sh_offset),		   
		   bss_section_index,
		   (uint32_t)(file_data + 
			      section_header[bss_section_index].sh_offset)) ;
  
  // Now, I have made the sanity checks, I know .text exists, and I
  // have to compute the relocation distance. Recall how our ELF file
  // was built:
  // - base address equal to 0
  // - first section is .text
  // - sections allocated consecutively
  // Under these hypotheses, all addresses are defined with respect
  // to the beginning of .text.
  //
  // If there are addresses to relocate (it may happen that this
  // is not the case), then relocation is done by adding to the
  // .got values the position of file_data plus the size of the
  // ELF header. This value must be a multiple of the word size, to
  // ensure alignment. This value is the same for all relocations
  // because I don't put the segments in different places. It may
  // change in the future, with .data and .bss somewhere else,
  // e.g. with a copy of .data kept for reset purposes and the
  // use copy together with .bss.
  uint32_t reloc_size =
    section_header[text_section_index].sh_offset -  // Actual position in file
    section_header[text_section_index].sh_addr +    // Addr used during compilation
    (uint32_t)file_data ;                           // Start of the memory region
  // Check alignment on ARM32 word size.
  if(reloc_size != ((reloc_size >> 2) << 2)) {
    return -9 ;
  }
  /*  elf_debug_printf("relocate_elf_file: reloc_size = 0x%x + 0x%x = 0x%x\n",
		   sizeof(struct Elf32Header),
		   (uint32_t)file_data,
		   reloc_size) ;*/
  elf_debug_printf("relocate_elf_file: reloc_size = 0x%x\n",
		   reloc_size) ;
  
  // Set the p_text_start output
  *p_text_start = elf32_header->e_entry + reloc_size ;
  elf_debug_printf("relocate_elf_file: text_start = %x \n",
		   *p_text_start) ;
  
  
  // Performing relocations.
  if(got_section_index<0) {
    // No .got section found, no relocation was needed.
    elf_debug_printf("relocate_elf_file: no relocation needed.\n") ;
  } else {
    // I have a .got section, I have to perform some relocations.

    // Change all values to the relocated ones.
    uint32_t* got_section =
      (uint32_t*)(file_data + 
		  section_header[got_section_index].sh_offset) ;
    int got_section_size = 
      section_header[got_section_index].sh_size / sizeof(uint32_t) ;
  
    for(i=0;i<got_section_size;i++) {
      uint32_t relocation = got_section[i] + reloc_size ;
      elf_debug_printf("\trelocate_elf_file: replacing .got pointer %x with %x\n",
		       got_section[i],
		       relocation) ;
      got_section[i] = relocation ;
    }
  }

  // Default values for the case there's no .bss
  *p_bss_base = *p_text_start ;
  *p_bss_size = 0 ;
  if(bss_section_index<0) {
    // No .bss section found, no relocation was needed.
    elf_debug_printf("\trelocate_elf_file: no BSS.") ;
  } else {
    // Record the position and size of the bss section (updated
    // for relocation).
    struct Elf32SectionHeader *bss_hdr = &section_header[bss_section_index] ;
    *p_bss_size = bss_hdr->sh_size ;
    *p_bss_base = bss_hdr->sh_addr + reloc_size ;
    // Print bss section header
    elf_debug_printf("relocate_elf_file: BSS header:\n") ;
    elf_debug_printf("\tsh_name = %s",section_name_base+bss_hdr->sh_name) ;
    elf_debug_printf("\tsh_type = ") ;
    if(bss_hdr->sh_type == SHT_NOBITS) {
      elf_debug_printf("SHT_NOBITS\n") ;
    } else {
      elf_debug_printf("BAD: %x\n",bss_hdr->sh_type) ;
    }
    elf_debug_printf("\tsh_flags     = %8x\n",bss_hdr->sh_flags);
    elf_debug_printf("\tsh_addr      = %8x, relocated to %8x\n",
		     bss_hdr->sh_addr,
		     *p_bss_base);
    elf_debug_printf("\tsh_offset    = %8x\n",bss_hdr->sh_offset);
    elf_debug_printf("\tsh_size      = %8x\n",bss_hdr->sh_size);
    elf_debug_printf("\tsh_link      = %8x\n",bss_hdr->sh_link);
    elf_debug_printf("\tsh_info      = %8x\n",bss_hdr->sh_info);
    elf_debug_printf("\tsh_addralign = %8x\n",bss_hdr->sh_addralign);
    elf_debug_printf("\tsh_entsize   = %8x\n",bss_hdr->sh_entsize);
  }
  elf_debug_printf("relocate_elf_file: bss_base = %x bss_size\n",
		   *p_bss_base,*p_bss_size) ;

  // Return without error
  return 0 ;
}


