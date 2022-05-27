
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
#ifndef REGISTERS_H
#define REGISTERS_H

//---------------------------------------------------------------
// Register use table for the arm5 (and the RPI), cf. 
//    http://www.pp4s.co.uk/main/tu-trans-asm-arm.html
//    http://www.coranac.com/tonc/text/asm.htm
//    ARM ARM
//---------------------------------------------------------------
// Reg      | Alternative | Description
//---------------------------------------------------------------
// r0 - r3  |          	  | Used to hold arguments for procedures 
//          |             | and as scratch registers (for 
//          |             | temporary storage). R0 is used to 
//          |             | return the result of a function.
//---------------------------------------------------------------
// r4 - r9  | v1 - v6 	  | General purpose or storage of 
//          |             | variables
//---------------------------------------------------------------
// r10 	    | sl, v7      | Stack limit pointer, used by 
//          |             | assemblers for stack checking when 
//          |             | this option is selected by the user.
//---------------------------------------------------------------
// r11      | fp, v8      | Frame pointer. From Jack Crenshaw's 
//          |             | section on local variables when 
//          |             | translating procedures, "Formal 
//          |             | parameters are addressed as positive 
//          |             | offsets from the frame pointer, and 
//          |             | locals as negative offsets".
//---------------------------------------------------------------
// r12      | ip          | Intra-Procedure-call scratch. Used 
//          |             | with r0 - r3 for temporary storage 
//          |             | and original contents do not need to 
//          |             | be preserved.
//---------------------------------------------------------------
// r13      | sp          | Stack pointer
//---------------------------------------------------------------
// r14      | lr          | Link register, holding the return 
//          |             | address from a function
//---------------------------------------------------------------
// r15      | pc          | Program counter, holding the address 
//          |             | of the next instruction
//---------------------------------------------------------------
//
// Special registers:
//---------------------------------------------------------------
//          | CPSR        | Current Program Status Register
//---------------------------------------------------------------
//          | SPSR        | Saved Program Status Registers
//          |             | Only in exception modes (Supervisor, 
//          |             | Abort, Interrupt, Fast Interrupt,
//          |             | Undefined)
//---------------------------------------------------------------

#if defined(ASSEMBLY_FILE) || defined(LDSCRIPT_FILE)

// Some constants for accessing very fast these definitions in
// assembly code.
#define CPSR_IRQ_INHIBIT     0x80 
#define CPSR_FIQ_INHIBIT     0x40

#else

// Regular C/C++ file, I include the headers allowing the
// definition of data structures.
#include <libc/stdint.h>

// Self-explaining.
uint32_t get_stack_pointer(void) ;

//---------------------------------------------------------------
// CPSR and SPSR structure (bit-encoded struct)
// From armv5 ARM, section A2.5.
// 
// User-writable bits:  N,Z,C,V,Q,GE,E
// Privileged bits: A,I,F,M (can be written in privileged modes)
// Execution state bits: J,T (should not be written)
//--------------------------------------------------------------- 
typedef union {
  uint32_t bv ;
  struct CPSR_decoder {
    //                /* bits   */ /* Meaning                      */
    int M:5 ;         /* 0..4   */ /* Processor mode               */
    int T:1 ;         /* 5      */ /* Thumb state flag (not RPI)   */
    int F:1 ;         /* 6      */ /* Disable Fast IRQs            */
    int I:1 ;         /* 7      */ /* Disable IRQs                 */
    int A:1 ;         /* 8      */ /* Disable imprecise data aborts*/
    int E:1 ;         /* 9      */ /* Endianness                   */
    int reserved1:6 ; /* 10..15 */
    int GE:4 ;        /* 16..19 */ /* Greater than or equal flags
				      for halfword/byte operations */
    int reserved2:4 ; /* 20..23 */
    int J:1 ;         /* 24     */ /* Jazelle state flag (not RPI) */
    int reserved3:2 ; /* 25..26 */
    int Q:1 ;         /* 27     */ /* Overflow/saturation in some 
				      DSP operations               */
    int V:1 ;         /* 28     */ /* Condition flag: underflow    */
    int C:1 ;         /* 29     */ /* Condition flag: carry        */
    int Z:1 ;         /* 30     */ /* Condition flag: zero         */
    int N:1 ;         /* 31     */ /* Condition flag: negative     */
  } decoder ;
} CPSR_type;

// I will use this data structure to save the context of a
// process. Not all registers are needed, but I put them here to
// have a full description.
struct FullRegisterSet{
  uint32_t r[13] ; // Registers r0-12
  uint32_t sp ;    // Register r13=sp
  uint32_t lr ;    // Register r14=lr
  uint32_t pc ;    // Register r15=pc
  CPSR_type cpsr ;  
};
/* This is a routine that gives control back to a piece of code
   based on a full context. Right now, this function is used,
   but in the final OS it will not. Indeed, it should only be used
   to give control back to tasks in user mode, but this is only
   done by the partition schedulers, which must run in user mode,
   and thus cannot and need not use ldm with ^ at the end.

   To give control to a partition I have to transfer it the
   context and then it can resume (and change the mode at the
   same time by using an spsr specifying user mode). I also
   have to provide it a Boolean to determine if I want to
   reboot the partition. */
void restore_context_and_resume(const struct FullRegisterSet* const rs) ;
// Print the full register set (no FPU, no NEON)
void PrintRegisterSet(const struct FullRegisterSet* const rs) ;



// Allow IRQs (not FIQs) to interrupt the processor.
void enable_interrupts(void) ;
void disable_interrupts(void) ;

#endif

//---------------------------------------------------------------
// Modes
// From armv5 ARM, section A2.5.
//---------------------------------------------------------------
// 
// From the ARM ARM (Architecture Reference Manual). Make sure you get the
// ARMv5 documentation which includes the ARMv6 documentation which is the
// correct processor type for the Broadcom BCM2835. The ARMv6-M manuals
// available on the ARM website are for Cortex-M parts only and are very
// different.
//
// See ARM sections A2.2 (Processor Modes), A2.5.7
#define CPSR_MODE_USER       0x10
#define CPSR_MODE_FIQ        0x11
#define CPSR_MODE_IRQ        0x12
#define CPSR_MODE_SVR        0x13
#define CPSR_MODE_ABORT      0x17
#define CPSR_MODE_UNDEFINED  0x1B
#define CPSR_MODE_SYSTEM     0x1F

//---------------------------------------------------------------
// Accessible registers, by mode
// From armv5 ARM, section A2.5.7
//---------------------------------------------------------------
// USER        | PC, R14-R0, CPSR   
// FIQ         | PC, R14_fiq-R8_fiq, R7 to R0, CPSR, SPSR_fiq
// IRQ         | PC, R14_irq, R13_irq, R12 to R0, CPSR, SPSR_irq
// SVR         | PC, R14_svc, R13_svc, R12 to R0, CPSR, SPSR_svc
// ABORT       | PC, R14_abt, R13_abt, R12 to R0, CPSR, SPSR_abt
// UNDEFINED   | PC, R14_und, R13_und, R12 to R0, CPSR, SPSR_und
// SYSTEM      | PC, R14 to R0, CPSR (ARMv4 and above)

//---------------------------------------------------------------
// Other interesting sources of information 
//---------------------------------------------------------------
// http://www.ic.unicamp.br/~celio/mc404-2013/
//          arm-manuals/ARM_exception_slides.pdf
// 


#endif
