
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
/* boot.S - assembly startup code */
	// The use of processor modes:
	// - Supervisor mode is active at boot time (RESET) and
	//   after software interrupts (SVC). Also, supervisor mode
	//   is entered by all interrupt handlers except FIQ and
	//   SVC just after the full context change.
	// - In all handlers except RESET, SVC, and FIQ, the
	//   handler starts by a full context change, followed
	//   by changing the mode to SVC and resetting the
	//   stack to KERNEL_STACK_BASE (no info is preserved from
	//   one call to the next). Due to this, the handlers are
	//   non-interruptible (the kernel is not reentrant).
	//   This may change in the future if I create an
	//   interrupt stack.
	// - SVC and FIQ do not need a full context save, because
	//   they return to calling point or to an already saved
	//   point in the program.
	// - The exception modes different from SVC are only used
	//   for a very short period before entering SVC.


	// - The system mode is the mode for the scheduler (to share
	//   the register set with the user mode). It is currently
	//   used only for non-preemptable code, but it will also
	//   be used for preemptable kernel code (e.g. drivers).
	//   For non-preemptable code, the stack address is the
	//   same as for the other modes, as explained in
	//   mmap.h.

	// - User mode is used for the execution of tasks.
	// Note: changing execution mode cannot be done using
	//   a C function, because the link register (lr=r14)
	//   is mode specific (upon mode change the return
	//   address becomes false).



	
// Definition of important macros.
#include "boot-macros.S"
	
// To keep this in the first portion of the binary.
.section ".text.boot"
 
// The global entry point, named _start, and the few functions
// I implemented in assembly for precision.
.globl _start
// The storage for a full execution context, comprising
// regular registers and CPSR. It does not (yet) include
// floating point or Neon registers.
.globl _context_save_struct
.globl _context_load_struct
.globl _context_load_asm
		
// Entry point for the kernel. For the PI, when this is
// called, the following registers will contain specific
// information:
// r15 -> should begin execution at 0x8000.
// r0  -> 0x00000000
// r1  -> 0x00000C42
// r2  -> 0x00000100 - start of ATAGS
// These values should be preserved for the C-level
// initialization part (I think there's some
// mailbox decoding to be done).
//	
// The code starts with the interrupt vector table, which
// points to the vectors (functions) of file interrupts.c.
_start:
	ldr pc, _reset_h
	ldr pc, _undefined_instruction_vector_h
	ldr pc, _svc_vector_h
	ldr pc, _prefetch_abort_vector_h
	ldr pc, _data_abort_vector_h
	ldr pc, _unused_handler_h
	ldr pc, _irq_vector_h
	ldr pc, _fiq_vector_h
// The following data are pointers to the handler functions.
_reset_h: 				.word _reset_asm_
_undefined_instruction_vector_h:	.word _undef_handler_asm_
_svc_vector_h: 				.word _svc_handler_asm_
_prefetch_abort_vector_h: 		.word _prefetch_abort_handler_asm_
_data_abort_vector_h: 			.word _data_abort_handler_asm_
_unused_handler_h: 			.word unused_exception_vector
_irq_vector_h: 				.word _irq_handler_asm_
_fiq_vector_h: 				.word _fiq_handler_asm_

/*=================================================*/
/* Reset handler, called when I want to reboot the */
/* system (actually, never, in the current         */
/* implementation).                                */
/* It is also used to fill in the unused vector.   */
/*=================================================*/
_reset_asm_:
	// The execution of the _reset_ vector is entered
	// in supervisor (SVR) mode. For more information on
	// processor modes see ARM Section A2.2 (Processor
	// Modes).
	//	
	// Start by saving the context, to allow its use by
	// the C routine (e.g. to recover the data transmitted
	// by the higer-level kernel. The parameter is not
	// important, because I will never have to load this
	// context. This call also sets the stack to
	//
	save_full_context 0

	// Place the interrupt vector table at address 0
	// by copying it from the start of this code.
	//
	// Source address (0x8000 for the PI B+) 
	mov r0, #KERNEL_ENTRY_POINT
	// Destination address (0x0)
	mov r1, #INTERRUPT_TABLE_ADDR
	// First, move the "ldr" statements that get executed. 
	ldmia r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
	stmia r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
	// Second, move the table of pointers used by the
	// "ldr" statements (the ldr statement uses a
	// local address).
	ldmia r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
	stmia r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
	
	// Clear out bss. which is set after the .data
	// segment. The symbols _bss_start and _bss_end
	// are defined in the loader script.
	//
	// First, set up the function call parameters.
	// Calling convention is ARM-specific: 
	// - r0 holds the first parameter (start pointer)
	// - r1 holds the length in bytes (unsigned int)
	// (note, this convention holds only for less than
	// 3 parameters.
	ldr	r0, =_bss_start
	ldr	r1, =_bss_end
	sub 	r1, r1, r0 
	// Call bzero
	ldr	r3, =bzero
	blx	r3 
	
	// Now, call the C startup file.
	ldr	r3, =kernel_init
	blx	r3
	// End with an infinite loop, which should never
	// get to execute after the first timer interrupt.
	// This is actually a function call, but since
	// we don't return there is no need to put the
	// forms...
	b	exit

/*================================================*/
/* IRQ handler, called when I receive an IRQ.     */
/*================================================*/
_irq_handler_asm_:
	// This is the handler of the IRQ interrupt.
	// When this vector is started, I assume that
	// interrutps are disabled, as explained in:
	//  http://www.ic.unicamp.br/~celio/mc404-2013/
	//               arm-manuals/ARM_exception_slides.pdf
	
	// First, save the execution context of the
	// interrupted task. This **must** remain the first
	// code to be executed.
	save_full_context LROFFSET_IRQ

	// Now, call the scheduling routine. This
	// one will have the task of:
	// - determining which task was executing
	// - copying the context saved above in the data structure
	//   of the task.
	// - choosing a task to execute
	// - enable interrupts and load the context (i.e. resume the
	//   task).
	// The scheduler may also create new tasks, etc.
	//
	// Note: Once SP and LR are correctly set, I can go to C
	// language programming by simply terminating the vector.
	ldr	r0, =irq_handler
	blx	r0

/*================================================*/
/* UNDEF handler, called when I receive an        */
/*	undefined instruction exception.          */
/*================================================*/
_undef_handler_asm_:
	// For the structure and meaning of this handler,
	// take a look at the IRQ handler above.
	save_full_context LROFFSET_UNDEFINED_INST	
	ldr 	r0, =undef_handler // defined in interrupts.c
	blx	r0
	
/*================================================*/
/* Prefetch abort handler, called when the MMU    */
/* cannot handle a jump (the destination is not   */
/* acceptable (bad address, bad permissions...).  */
/*================================================*/
_prefetch_abort_handler_asm_:
	// For the structure and meaning of this handler,
	// take a look at the IRQ handler above.
	save_full_context LROFFSET_PREFETCH_ABORT
	// Place in r0, r1, and r2 information about
	// who caused the prefetch abort exception, and give
	// these to function prefetch_abort_handler. According
	// to ARM EABI, r0, r1, r2 will be seen as the
	// 3 parameters of the function call.
        //
	// All these data are taken from the ARM1176JZF-S
	// Technical Reference Manual. Attention, other versions
	// of the processor (e.g. MPCore) have other register
	// encodings.
	
	// Parameter 1: Instruction Fault Status Register
	MRC	p15, 0, r0, c5, c0, 1
	// Parameter 2: Instruction fault address register
	MRC 	p15, 0, r1, c6, c0, 2
	// Parameter 3: Fault address register
	MRC	p15, 0, r2, c6, c0, 0 
	ldr 	r3, =prefetch_abort_handler
	blx	r3
	
/*================================================*/
/* Data abort handler, called when the MMU        */
/* cannot handle a data access (the destination   */
/* is not acceptable.                             */
/*================================================*/
_data_abort_handler_asm_:
	// For the structure and meaning of this handler,
	// take a look at the IRQ handler above.
	save_full_context LROFFSET_DATA_ABORT
	// Place in r0 and r1 information about
	// who caused the data abort exception, and give
	// these to function data_abort_handler. According
	// to ARM EABI, r0 and r1 will be seen as the
	// 2 parameters of the function call.
        //
	// All these data are taken from the ARM1176JZF-S
	// Technical Reference Manual. Attention, other versions
	// of the processor (e.g. MPCore) have other register
	// encodings.
	
	// Parameter 1: Data Fault Status Register
	MRC 	p15, 0, r0, c5, c0, 0
	// Parameter 2: Fault address register
	MRC	p15, 0, r1, c6, c0, 0
	ldr 	r3, =data_abort_handler
	blx	r3

/*================================================*/
/* Software interrupt handler.                    */
/*================================================*/
_svc_handler_asm_:
	// In the beginning, I hoped this would be a
	// simpler context save, closer to a function
	// call protocol. But some ARINC 653 primitives,
	// such as PERIODIC_WAIT require a context change.
	// Thus, I need a full context change, like for
	// IRQs.
	//
	//	
	// First, save the execution context of the
	// interrupted task. This **must** remain the first
	// code to be executed.
	save_full_context LROFFSET_SVC

	// Register r0 can be modified by save_full_context.
	// I need to restore it.
	ldr r0,_r0_cs
	
	// Now, call the scheduling routine. This
	// one will have the task of:
	// - determining which task was executing
	// - copying the context saved above in the data structure
	//   of the task.
	// - choosing a task to execute
	// - enable interrupts and load the context (i.e. resume the
	//   task).
	// The scheduler may also create new tasks, etc.
	//
	// Note: Once SP and LR are correctly set, I can go to C
	// language programming by simply terminating the vector.
	ldr	r4, =svc_handler
	blx	r4
	//	
	// Now, return to the caller by reloading just the
	// needed registers (take advantage of SP, LR, CPSR
	// being cached, and do not reload r0-r3, which are
	// return values under ARM EABI). Attention, there is no
	// need to restore the SPSR, we use the one that was saved.
	ldr 	sp, =_r4_cs
	ldmia	sp, {r4-r12}
	ldr	sp, =_pc_cs
	ldmia	sp, {pc}^

	
/*================================================*/
/* FIQ interrupt handler.                         */
/*================================================*/
_fiq_handler_asm_:
	// This is the second handler I built under the
	// assumption that no full context save is needed.
	// Furthermore, for FIQ I assume the handler always
	// completes.
	mov 	sp, #KERNEL_STACK_BASE
	// Store unbanked context on the stack.
	push	{r0-r7,lr}
	// Call the C handler.
	ldr 	r0, =fiq_handler
	blx	r0
	// Return to caller.
	LDMIA	sp, {R0-R7,pc}^
	
/*================================================*/
/* Context load routine.                          */
/*================================================*/

_context_load_asm:
	// Assume I'm in an exception mode and that
	// I have to enter either SYSTEM or USER
	// mode. I first have to place SP and LR on the
	// banked registers.
	entermode CPSR_MODE_SYSTEM
	ldr	sp, _sp_cl
	ldr	lr, _lr_cl
	// Then, I come back, because otherwise it's not
	// possible to do the final ldmia.
	entermode CPSR_MODE_SVR
	ldr     r0, _cpsr_cl
	msr	spsr, r0
	ldr 	r0, =_r0_cl
	ldmia	r0, {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}
	// To load PC, I will use SP as a base register.
	// It's being thrown away, anyway.
	ldr	sp, =_pc_cl
	ldmia	sp, {pc}^
	
/* I write this data structure in assembly to facilitate
   direct access in assembly to the various fields. It can be
   read in C using the struct FullRegisterSet defined in
   registers.h.
 */
	
_context_save_struct:
_r0_cs:		.word	0
_r1_cs:		.word	0
_r2_cs:		.word	0
_r3_cs:		.word	0
_r4_cs:		.word	0
_r5_cs:		.word	0
_r6_cs:		.word	0
_r7_cs:		.word	0
_r8_cs:		.word	0
_r9_cs:		.word	0
_r10_cs:	.word	0
_r11_cs:	.word	0
_r12_cs:	.word	0
_sp_cs:		.word	0
_lr_cs:		.word	0
_pc_cs:		.word	0
_cpsr_cs:	.word	0

_context_load_struct:
_r0_cl:		.word	0
_r1_cl:		.word	0
_r2_cl:		.word	0
_r3_cl:		.word	0
_r4_cl:		.word	0
_r5_cl:		.word	0
_r6_cl:		.word	0
_r7_cl:		.word	0
_r8_cl:		.word	0
_r9_cl:		.word	0
_r10_cl:	.word	0
_r11_cl:	.word	0
_r12_cl:	.word	0
_sp_cl:		.word	0
_lr_cl:		.word	0
_pc_cl:		.word	0
_cpsr_cl:	.word	0


	
