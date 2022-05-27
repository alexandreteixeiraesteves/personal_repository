// This code was used to test the elf.c file.
// It is  no longer maintained, but it's still here for
// future debug needs.

/*
    // Check that section 1 is .text
    // Check that the start of the text section is the start of the
    // file data.
    // The relocation distance corresponds to the difference
    // between the file start and the start of .text. It is the
    // same for all relocations because I don't put the segments
    // in different places. It may change in the future, with
    // .data and .bss somewhere else, e.g. with a copy of .data
    // kept for reset purposes and the use copy together with
    // .bss.
    uint32_t reloc_size = elf32_header->e_entry-(uint32_t)file_data ;
    


  struct Elf32ProgramHeader* pheader = 
    (struct Elf32ProgramHeader*)(file_data+elf32_header->e_phoff) ;
  // Now, move all the data starting from the start of .text at address
  // file_data.
  {
    uint32_t size = pheader[0].p_filesz ;
    elf_debug_printf("Moving %x bytes of data from %x to %x\n",
		     pheader[0].p_filesz,
		     file_data+section_header[1].sh_offset,
		     file_data);
    memcpy(file_data,                               // destination
	   file_data+section_header[1].sh_offset,   // source
	   pheader[0].p_filesz) ;                   // size
    //Now, zero the bss section
    bzero((char*)bss_section_start,bss_section_size) ;
    return size ;
  }
  */

/*
int main() {
  const char elf_file_name[] = "parti.elf" ;
  FILE* elf_file = fopen(elf_file_name,"r") ;
  if(elf_file==NULL) {
    printf("No such file: %s\n",elf_file_name) ;
    return 1 ;
  }
  fseek(elf_file, 0, SEEK_END);
  uint32_t size = ftell(elf_file);
  fseek(elf_file, 0, SEEK_SET);
  char* file_data ;
  printf("ELF file exists with length %u\n",size) ;
  // Can't call memalign with alignment smaller than 
  // sizeof(void*).
  if(posix_memalign((void**)&file_data,sizeof(void*),size)!=0){
    printf("Cannot allocate memory buffer\n",size) ;
    return 1 ;
  }
  if(fread(file_data,1,size,elf_file)!=size) {
    printf("Could not read the entire file.\n") ;
    return 1 ;
  }
  struct Elf32Header* elf32_header = (struct Elf32Header*)file_data ;
  printf("ELF file header:\n"
	 "\tIdentifier:\n"
	 "\t\tMagic: 0x%2x %c%c%c (should be 0x7f ELF)\n"
	 "\t\tClass:0x%x (1=ELF32bit) Endianness:0x%x (1=little)   Version:0x%x\n"
	 "\t\tABI_Type:0x%x (0=System V Unix) ABI_Version:0x%x\n"
	 "\tType:0x%x (2=executable)\n"
	 "\tMachine:0x%x (0x28=ARM)\n"
	 "\tVersion:0x%x\n"
	 "\tEntry:0x%x\n"
	 "\tProgram header offset:0x%x\n"
	 "\tSection header offset:0x%x\n"
	 "\tFlags:0x%x\n"
	 "\tELF header size: 0x%x (should be 0x34)\n"
	 "\tProgram header size: 0x%x (should be 0x20)\n"
	 "\tNumber of program headers: 0x%x (should be 1)\n"
	 "\tSection header size: 0x%x (should be 0x28)\n"
	 "\tNumber of section headers: 0x%x\n"
	 "\tSection names are found in section: 0x%x\n",
	 elf32_header->e_ident.magic[0],
	 elf32_header->e_ident.magic[1],
	 elf32_header->e_ident.magic[2],
	 elf32_header->e_ident.magic[3],
	 elf32_header->e_ident.class,
	 elf32_header->e_ident.endianness,
	 elf32_header->e_ident.version,
	 elf32_header->e_ident.type,
	 elf32_header->e_ident.ABI_version,
	 elf32_header->e_type,
	 elf32_header->e_machine,
	 elf32_header->e_version,
	 elf32_header->e_entry,
	 elf32_header->e_phoff,
	 elf32_header->e_shoff,
	 elf32_header->e_flags,
	 elf32_header->e_ehsize,
	 elf32_header->e_phentsize,
	 elf32_header->e_phnum,
	 elf32_header->e_shentsize,
	 elf32_header->e_shnum,
	 elf32_header->e_shstrndx) ;
  uint32_t i ;
  struct Elf32ProgramHeader* pheader = 
    (struct Elf32ProgramHeader*)(file_data+elf32_header->e_phoff) ;
  for(i=0;i<elf32_header->e_phnum;i++){
    printf("Program header[%u]=\n"
	   "\tType of program header entry:0x%x\n"
	   "\tOffset in the file:0x%x\n"
	   "\tVirtual address:0x%x\n"
	   "\tPhysical address:0x%x\n"
	   "\tFile size:0x%x\n"
	   "\tMemory size:0x%x\n"
	   "\tFlags: X:%d W:%d R:%d OS_Mask:0x%x ISA_Mask:0x%x\n"
	   "\tAlignment:0x%x\n",
	   i,
	   pheader[i].p_type ,
	   pheader[i].p_offset,
	   pheader[i].p_vaddr,
	   pheader[i].p_paddr,
	   pheader[i].p_filesz,
	   pheader[i].p_memsz,
	   pheader[i].p_flags.X,
	   pheader[i].p_flags.W,
	   pheader[i].p_flags.R,
	   pheader[i].p_flags.OS_Mask,
	   pheader[i].p_flags.ISA_Mask,
	   pheader[i].p_align) ;
  }

  struct Elf32SectionHeader* section_header =
    (struct Elf32SectionHeader*)(file_data+elf32_header->e_shoff) ;
  // Check the consistency of the strtab section choice.
  if((elf32_header->e_shstrndx <= 0)||
     (elf32_header->e_shstrndx > elf32_header->e_shnum)) {
    printf("\tBad section index for the section header names. "
	   "Aborting...\n") ;
    exit(-1) ;
  }
  printf("Found STRTAB section number: %x\n",
	 elf32_header->e_shstrndx) ;
  char* section_name_base = 
    (char*)(file_data+
	    section_header[elf32_header->e_shstrndx].sh_offset) ;
  
  // STRTAB found. Now, print the segment headers.
  for(i=0;i<elf32_header->e_shnum;i++){
    printf("Section %u: \"%s\" ",
	   i,
	   section_name_base+section_header[i].sh_name) ;
    printf("(") ;
    if(section_header[i].sh_type >= SHT_ERROR) {
      if(section_header[i].sh_type>=SHT_LOPROC){
	if(section_header[i].sh_type<SHT_HIPROC) {
	  printf("PROC-SPECIFIC (%x)",section_header[i].sh_type) ;
	} else {
	  printf("APP-SPECIFIC (%x)",section_header[i].sh_type) ;
	}
      } else {
	printf("invalid type (%x). "
	       "Aborting...\n",
	       section_header[i].sh_type) ;
	exit(-1) ;
      }
    } else {
      printf("%s",SectionTypeNames[section_header[i].sh_type]) ;
    }
    printf(")  ") ;
    if(*((uint32_t*)&(section_header[i].sh_flags))!=0) {
      printf("Flags: %s%s%s%s%s%s%s%s%s%s",
	     (section_header[i].sh_flags.SHF_WRITE?"write ":""),
	     (section_header[i].sh_flags.SHF_ALLOC?"alloc ":""),
	     (section_header[i].sh_flags.SHF_EXECINSTR?"execinst ":""),
	     (section_header[i].sh_flags.SHF_MERGE?"merge ":""),
	     (section_header[i].sh_flags.SHF_STRINGS?"strings ":""),
	     (section_header[i].sh_flags.SHF_INFO_LINK?"infolink ":""),
	     (section_header[i].sh_flags.SHF_LINK_ORDER?"linkorder ":""),
	     (section_header[i].sh_flags.SHF_OS_NONCONFORMING?"os_nonconf ":""),
	     (section_header[i].sh_flags.SHF_GROUP?"group ":""),
	     (section_header[i].sh_flags.SHF_TLS?"tls ":"")) ;
      if(section_header[i].sh_flags.SHF_MASKOS) {
	printf("MaskOS(%x) ",section_header[i].sh_flags.SHF_MASKOS) ;
      }
      if(section_header[i].sh_flags.SHF_MASKPROC) {
	printf("MaskProc(%x) ",section_header[i].sh_flags.SHF_MASKPROC) ;
      }
    }
    printf("\n") ;
    printf("\tMemAddr: %8x"
	   "\tFileOff: %8x"
	   "\tSize: 0x%x"
	   "\tLink: 0x%x"
	   "\tInfo: 0x%x"
	   "\tAlign: 0x%x\n"
	   "\tEntry size if table:0x%x\n",
	   section_header[i].sh_addr,
	   section_header[i].sh_offset,
	   section_header[i].sh_size,
	   section_header[i].sh_link,
	   section_header[i].sh_info,
	   section_header[i].sh_addralign,
	   section_header[i].sh_entsize) ;
  }
  
  
  // Find the .got section
  int got_section_index = -1 ;
  int got_plt_section_index = -1 ;
  for(i=0;i<elf32_header->e_shnum;i++){
    if(strcmp(section_name_base+section_header[i].sh_name,".got")==0){
      got_section_index = i ;
    }
    if(strcmp(section_name_base+section_header[i].sh_name,".got.plt")==0){
      got_plt_section_index = i ;
    }
  }
  printf(".got section index: %x\n",got_section_index) ;
  printf(".got.plt section index: %x\n",got_plt_section_index) ;
  printf("Contents of .got section: \n") ;
  uint32_t* got_section =
    (uint32_t*)(file_data + 
		section_header[got_section_index].sh_offset) ;
  int got_section_size = 
    section_header[got_section_index].sh_size / sizeof(uint32_t) ;
  for(i=0;i<got_section_size;i++) {
    printf("\t%8x\n",got_section[i]) ;
  }
  printf("Contents of .got.plt section: \n") ;
  uint32_t* got_plt_section =
    (uint32_t*)(file_data + 
		section_header[got_plt_section_index].sh_offset) ;
  int got_plt_section_size = 
    section_header[got_plt_section_index].sh_size / sizeof(uint32_t) ;
  for(i=0;i<got_plt_section_size;i++) {
    printf("\t%8x\n",got_plt_section[i]) ;
  }
}
*/
