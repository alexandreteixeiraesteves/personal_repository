











 


























 
SECTIONS
{
    
    . = 0x8000;
    
    _text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    
    . = ALIGN(4);
    _text_end = .;
 
    _rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    
    . = ALIGN(4);
    _rodata_end = .;
 
    _data_start = .;
    .data :
    {
        *(.data)
    }
    
    . = ALIGN(4);
    _data_end = .;
 
    
    _bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    }
    
    . = ALIGN(4);
    _bss_end = .;
    _end = .;
}
