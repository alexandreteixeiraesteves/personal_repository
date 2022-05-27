
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
#ifndef MMAP_H
#define MMAP_H

/*-------------------------------------------------*/
/* First, some global definitions.                 */
/*-------------------------------------------------*/

/* Memory size. I set the global RAM size to 512Mo,
   and then set the first 256Mo to the ARM. Currently,
   I could allocate everything to the ARM because
   I don't use the GPU, but who knows what we'll do in
   the future ? 
*/
#define ARM_RAM_SIZE        0x10000000
#define GLOBAL_RAM_SIZE     0x20000000

/* Page size, for the MMU. 
   The first page(s) will be reserved for the system.
   The following pages are assigned to the partitions.
   This page size is the grain with which I will 
   create partitioning barriers between partitions.

   I set the page size to 1MByte, which makes for
   4096 pages in the whole 4GBytes address space 
   that can be addressed using the 32-bit addresses
   of the ARM (the page table of the MMU must have 
   4096 entries).
*/
#define PAGE_SIZE     0x100000
#define PAGE_NUMBER   0x1000

/* The alignment of segments in the ld script.
   If this is set to PAGE_SIZE, then I can really 
   set some segments to be read-only through the MMU, 
   such as the text or rodata segments.
   But aligning on page size creates a huge executable
   (>2Mbytes) and also means that I have to take care where 
   I put my heap and other data, because the executable 
   goes upper in the RAM.
   So, I only align on word. 
*/ 
#define SEGMENT_ALIGNMENT 4



/*-------------------------------------------------*/
/* Memory map for the Raspberry Pi B+              */
/* Physical addresses.                             */
/* Note:                                           */
/*    Bus addresses, as defined in the SoC manual, */
/*    can also be used prior to MMU configuration  */
/*    This means that address 0x00002000, address  */
/*    0x40002000, and address 0xc0002000 point to  */
/*    the same memory cell. There will be no error */
/*    raised when using them.                      */
/* Note 2: After MMU config only accesses to the   */
/*    low memory areas are accepted.               */
/*-------------------------------------------------*/

/* The kernel is allocated a total of 4Mbytes of
   memory:
   * The first is for code, data, and fixed 
     data structures. 
   * The second is for the stack of the kernel 
     itself (non-interruptible code). Whenever
     such a piece of code is entered (through an
     interrupt), the stack is reset to the base of 
     this segment.
   * The third is the stack of the system 
     partition (function system_partition_L2_scheduler).
     Later, this may include more complex
     preemptive driver code running in SYSTEM mode.
     At the end, it also includes the L1PartitionInterface
     data structure.
   * Finally, the last 1Mo segment is the heap.
     Attention, for consistency issues heap 
     reservations must be made using 
     non-interruptible code.
   As I have a lot of space, I don't try to 
   optimize, but simply keep the code simple. */
#define KERNEL_CODE_BASE      0x000000
#define KERNEL_CODE_SIZE      0x100000
#define KERNEL_CODE_END       (KERNEL_CODE_BASE+KERNEL_CODE_SIZE)
#define KERNEL_STACK_SIZE     0x100000
/* The kernel stack base could be defined with the
   following formula, but limitations of gnu as mean
   that I have to provide the value.
   #define KERNEL_STACK_BASE     (KERNEL_CODE_END+KERNEL_STACK_SIZE)
*/
#define KERNEL_STACK_BASE     0x200000
#define SYSTEM_STACK_SIZE     0x100000
#define SYSTEM_STACK_BASE     (KERNEL_STACK_BASE+SYSTEM_STACK_SIZE)
#define KERNEL_HEAP_BASE      SYSTEM_STACK_BASE
#define KERNEL_HEAP_SIZE      0x100000
#define KERNEL_END            (KERNEL_HEAP_BASE+KERNEL_HEAP_SIZE)

/* The interrupt vector table is at address 0.  It 
   contains 8 pointers (one unused), cf.  ARM ARM
   https://www.scss.tcd.ie/~waldroj/3d1/arm_arm.pdf 
   (for the v5, which covers this aspect of the Pi's 
   v6).
   Its size is set to twice this size because I need
   to have a set of jump addresses just afterwards
   (take a look into boot.S).
*/
#define INTERRUPT_TABLE_ADDR    0
#define INTERRUPT_TABLE_LENGTH  (2*8*4)

/* The loader of the Raspberry Pi first loads our
   kernel (kernel.img) at address 0x8000, and then
   gives control to the KERNEL_ENTRY_POINT address 
   (0x8000). This means that we must ensure that the 
   entry symbol _start of boot.S fits precisely at 
   that address.  

   I put nothing in the space between the interrupt
   table and the entry point (I could, but I'd like to
   avoid nasty memory errors). I could even render 
   part of the text segment read-only at some point.
*/
#define KERNEL_ENTRY_POINT 0x8000

/* Start of the memory-mapped peripherals (just after
   the RAM).
*/
#define PERIPHERALS_BASE                            0x20000000
#define SYSTIMER_BASE             (PERIPHERALS_BASE+  0x003000)
#define INTERRUPT_CONTROLLER_BASE (PERIPHERALS_BASE+  0x00B200)
#define ARMTIMER_BASE             (PERIPHERALS_BASE+  0x00B400)
#define MBOX_BASE                 (PERIPHERALS_BASE+  0x00b880)
#define GPIO_BASE                 (PERIPHERALS_BASE+  0x200000)
#define UART0_BASE                (PERIPHERALS_BASE+  0x201000)
#define EMMC_BASE                 (PERIPHERALS_BASE+  0x300000)
#define PERIPHERALS_END                             0x20400000

#endif
