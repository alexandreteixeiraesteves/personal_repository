
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
#ifndef ELF_INTERNALS_H
#define ELF_INTERNALS_H

#include <libc/stdint.h>

// The type of section 
enum SectionType{
  SHT_NULL     = 0x0, /* Empty section of index 0. */
  SHT_PROGBITS = 0x1, /* Text and data. */
  SHT_SYMTAB   = 0x2, /* Symbol table. */
  SHT_STRTAB   = 0x3, /* String table (stores the section names). */
  SHT_RELA     = 0x4,
  SHT_HASH     = 0x5,
  SHT_DYNAMIC  = 0x6,
  SHT_NOTE     = 0x7,
  SHT_NOBITS   = 0x8,
  SHT_REL      = 0x9,
  SHT_SHLIB    = 0xa,
  SHT_DYNSYM   = 0xb,
  SHT_ERROR    = 0xc, /* If the type is >= and not in the below
			 ranges, it's an error. */
  SHT_LOPROC   = 0x70000000,
  SHT_HIPROC   = 0x7fffffff,
  SHT_LOUSER   = 0x80000000,
  SHT_HIUSER   = 0xffffffff
} ;

const char* SectionTypeNames[] = {
  "NULL",
  "PROGBITS",
  "SYMTAB",
  "STRTAB",
  "RELA",
  "HASH",
  "DYNAMIC",
  "NOTE",
  "NOBITS",
  "REL",
  "SHLIB",
  "DYSYM"
} ;

// 32-bit-wide bitfield.
// I prefer using this sort of access to flags.
struct SectionAttributeFlags{
  uint32_t SHF_WRITE:1 ;  
  uint32_t SHF_ALLOC:1 ;  
  uint32_t SHF_EXECINSTR:1 ;
  uint32_t reserved1:1 ; 
  uint32_t SHF_MERGE:1 ;
  uint32_t SHF_STRINGS:1 ;
  uint32_t SHF_INFO_LINK:1 ;
  uint32_t SHF_LINK_ORDER:1 ;
  uint32_t SHF_OS_NONCONFORMING:1 ;
  uint32_t SHF_GROUP:1 ;
  uint32_t SHF_TLS:1 ;
  uint32_t reserved2:9 ;
  uint32_t SHF_MASKOS:8 ;
  uint32_t SHF_MASKPROC:4 ;
} ;

struct Elf32SectionHeader {
  uint32_t                     sh_name;	/* Section name. Index into the section
				           header string table. */
  enum SectionType             sh_type : 32;	  /* SHT_... */
  struct SectionAttributeFlags sh_flags ; /* SHF_... */
  uint32_t	sh_addr;	/* virtual address */
  uint32_t	sh_offset;	/* file offset */
  uint32_t	sh_size;	/* section size */
  uint32_t	sh_link;	// Interpretation depends on the type of
                                // segment. For a SYMTAB it gives the 
                                // STRTAB section index containing 
                                // section names.
  uint32_t	sh_info;	/* misc info */
  uint32_t	sh_addralign;	/* memory alignment */
  uint32_t	sh_entsize;	/* entry size if table */
} ;

enum ElfProgramType {
  PT_NULL    = 0,	       
  PT_LOAD    = 1,
  PT_DYNAMIC = 2,
  PT_INTERP  = 3,
  PT_NOTE    = 4,
  PT_SHLIB   = 5,
  PT_PHDR    = 6,
  PT_TLS     = 7,
  PT_NUM     = 8,
  PT_ERROR   = 0xffffffff  /* This last value is needed to ensure that
			      the magnitude of the type is 32 bits. */
};

// Program segment permissions, in program header p_flags field.
struct ProgramHeaderFlags {
  uint32_t X:1 ;
  uint32_t W:1 ;
  uint32_t R:1 ;
  uint32_t unused:17 ;
  uint32_t OS_Mask:8 ;
  uint32_t ISA_Mask:4 ;
};

struct Elf32ProgramHeader {
  enum ElfProgramType p_type:32 ; // Type of program header entry
  uint32_t p_offset ; // Offset in the file.
  uint32_t p_vaddr ;  // Virtual address
  uint32_t p_paddr ;  // Physical address
  uint32_t p_filesz ; // File size
  uint32_t p_memsz ;  // Memory size
  struct ProgramHeaderFlags p_flags ;
  uint32_t p_align ;  // Memory/File alignment
} ;

enum OS_ABI_Type{
  OS_SystemV = 0x00 ,   // Also used for none
  OS_HP_UX   = 0x01 ,
  OS_NetBSD  = 0x02 ,
  OS_Linux   = 0x03 ,
  OS_Hurd    = 0x04 ,
  OS_Solaris = 0x06 ,
  OS_AIX     = 0x07 ,
  OS_IRIX    = 0x08 ,
  OS_FreeBSD = 0x09 ,
  OS_TRU64   = 0x0a ,
  OS_MODESTO = 0x0b ,   // Novell Modesto
  OS_OpenBSD = 0x0c , 
  OS_OpenVMS = 0x0d ,   // OpenVMS
  OS_NSK     = 0x0e ,   // Hewlett-Packard Non-Stop Kernel
  OS_AROS    = 0x0f ,   // Amiga Research OS
  OS_ARM     = 0x61 ,
  OS_Standalone = 0xff  // Embedded application
} ;

enum ElfType{
  ELF_None        = 0x00,
  ELF_Relocatable = 0x01,
  ELF_Executable  = 0x02,
  ELF_Shared      = 0x03,
  ELF_Core        = 0x04,
  ELF_LoOS        = 0xfe00, // OS-specific
  ELF_HiOS        = 0xfeff, // OS-specific
  ELF_LoProc      = 0xff00, // Processor-specific
  ELF_HiProc      = 0xffff  // Processor-specific
};

// For more machine identifiers, take a look at 
// http://www.opensource.apple.com/source/gdb/gdb-1515/src/include/elf/common.h
enum ElfISA{
  ISA_SPARC    = 0x02 , 
  ISA_386      = 0x03 ,
  ISA_68K      = 0x04 , // Motorola m68k family
  ISA_MIPS     = 0x08 ,
  ISA_PowerPC  = 0x14 ,
  ISA_ARM      = 0x28 ,
  ISA_IA_64    = 0x32 ,
  ISA_x86_64   = 0x3e ,
  ISA_ERROR    = 0xffff  /* This value is needed to ensure that 
			    the magnitude of the type is 16 bits. */
};

struct Elf32Header {
  struct ElfHeaderIdentifier{
    uint8_t magic[4] ;   // Should be 0x7f followed by "ELF"
    uint8_t class ;      // Should be 1 for 32-bit ELF (2 for 64-bit)
    uint8_t endianness ; // Should be 1 for little endian (2 for big)
                         // Affects interpretation of fields starting 
                         // with field e_type.
    uint8_t version ;    // Must be 1 for original ELF
    enum OS_ABI_Type type:8 ; // OS ABI identifier. Often set to 0 
                              // (SystemV) regardless of OS. 
    uint8_t ABI_version; // Further spec. the ABI version
    uint8_t padding[7];  // Unused
  } e_ident ;
  enum ElfType e_type : 16 ;   // File type, should be Relocatable
  enum ElfISA  e_machine :16 ; // Target ISA, should be ARM
  uint32_t e_version ; // Set to 1 for the original ELF version
  // Start address (uint64_t for 64-bit ELF).
  // It's in fact a "struct Elf32ProgramHeader*", but to execute on
  // a 64-bit machine I have to declare it this way.
  uint32_t e_entry ;  
  // Pointer to the program header table usually situated just after
  // the header.
  // It's in fact a "struct Elf32ProgramHeader*", but to execute on
  // a 64-bit machine I have to declare it this way.
  uint32_t  e_phoff ; 
  // Pointer to the section header table.
  // It's in fact a "struct Elf32ProgramHeader*", but to execute on
  // a 64-bit machine I have to declare it this way.
  uint32_t e_shoff ; 
  uint32_t e_flags ; // Interpretation is architecture-dependent.
  uint16_t e_ehsize ;// Header size (52 bytes for 32-bit ELF).
  uint16_t e_phentsize;	 // Program header size.
  uint16_t e_phnum ; // Number of entries in the program header table.
  uint16_t e_shentsize ; // Size of the section header table.
  uint16_t e_shnum ; // Number of entries in the section header table.
  uint16_t e_shstrndx ;	// Index of the section header table entry that
                        // contains the section names.
};



#endif
