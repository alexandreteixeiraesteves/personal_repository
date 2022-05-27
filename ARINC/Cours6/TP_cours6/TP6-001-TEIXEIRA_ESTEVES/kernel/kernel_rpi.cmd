/* The memory map contains constants used to place
   the various segments in memory. The definition is
   needed to ensure that function definitions are not
   included in the ldscript. */
#define LDSCRIPT_FILE
#include <librpi/mmap.h>
 
SECTIONS
{
    /* Starts at KERNEL_ENTRY_POINT, it's mandatory because 
       load is done at this address and execution starts from
       it. On the existing RPIs, this address is 0x8000. */
    . = KERNEL_ENTRY_POINT;
    /* All segments provide _segment_start and 
       _segment_end symbols that can be used during 
       execution (e.g. to set up .bss). */
    _text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
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
        bss = .;
        *(.bss)
    }
    /* Align segment end. See mmap.h for more info. */
    . = ALIGN(SEGMENT_ALIGNMENT);
    _bss_end = .;
    _end = .;
}
