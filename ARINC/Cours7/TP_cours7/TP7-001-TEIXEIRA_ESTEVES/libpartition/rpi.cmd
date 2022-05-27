/* The memory map contains constants used to place
   the various segments in memory. The definition is
   needed to ensure that function definitions are not
   included in the ldscript. */
#include <librpi/mmap.h>

/* This is the entry symbol of the kernel. Placing the
   directive here has no consequence on the loading
   process, because this process is fixed: load the
   image at START_ADDR, then branch to START_ADDR. The entry
   symbol is therefore not respected, as it would be 
   if an ELF file is loaded in a classical OS. 
   However, I let it here because it reminds people 
   where the entry is... (the symbol is defined in 
   boot.S. */
ENTRY(L2_entry_point)
 
SECTIONS
{
    /* Starts at 0, to facilitate relocation. */
    . = 0 ;
    /* All segments provide _segment_start and 
       _segment_end symbols that can be used during 
       execution (e.g. to set up .bss). */
    _text_start = . ;
    .text :
    {
	KEEP(*(.text.l2entrypoint))
        *(.text)
    }
    /* Align segment end. See mmap.h for more info. */
    . = ALIGN(SEGMENT_ALIGNMENT);
    _text_end = .;
 
    _rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    /* Align segment end. See mmap.h for more info. */
    . = ALIGN(SEGMENT_ALIGNMENT);
    _rodata_end = .;
 
    _data_start = .;
    .data :
    {
        *(.data)
    }
    /* Align segment end. See mmap.h for more info. */
    . = ALIGN(SEGMENT_ALIGNMENT);
    _data_end = .;
 
    /* .bss is at the end of the file because it corresponds
       to no actual loaded code, but will need to be 
       initialized (zeroed) before execution. */
    _bss_start = .;
    .bss :
    {
        KEEP(*(.bss.interface))
        *(.bss)
    }
    /* Align segment end. See mmap.h for more info. */
    . = ALIGN(SEGMENT_ALIGNMENT);
    _bss_end = .;
    _end = .;
}
