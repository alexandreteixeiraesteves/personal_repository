
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
#define ASSEMBLY_FILE
	
#include <librpi/mmap.h>
#include <librpi/registers.h>

.macro entermode procmode
	// Enter procmode. This macro is only called in the
	// non-interruptible kernel part, so I take care to keep
	// interrupts (IRQs and FIQs) disabled.
	// Uses and modifies r0 and cpsr.
	mov 	r0, #(\procmode | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
	msr 	cpsr_c, r0	
.endm

// Link register offsets (LRO).
// This is the offset that needs to be subtracted
// from LR to get the correct return value for the various
// exception types. It depends on
// the type of interrupt we handle, cf.
//  http://www.ic.unicamp.br/~celio/mc404-2013/
//               arm-manuals/ARM_exception_slides.pdf
// LRO is not defined for RESET.
#define LROFFSET_DATA_ABORT		8
#define LROFFSET_FIQ			4
#define LROFFSET_IRQ			4
#define LROFFSET_PREFETCH_ABORT		4
#define LROFFSET_SVC			0
#define LROFFSET_UNDEFINED_INST		0
	
// Full context save macro for interrupts different from FIQ and SWI.
// It saves the context, switches to SVR mode, and sets the stack.
// It assumes that interrupted code is in either USER or SYSTEM mode. 	
// The macro modifies register r0
.macro save_full_context lroffset
	// Should be called right at the beginning of interrupt
	// handlers.
	// The state that is saved contains:
	// - registers r0-r15 (sp and lr recovered from banked
	//   user mode registers).
	// - the control register CPSR, with all the flags
	//   inside it.
	// This macro may be extended to include information on
	// the source of the interrupt.
	//	
	// State is saved in the data structure
	// _context_save_struct, defined below, which can be
	// read using the C definition "struct FullRegisterSet"
	// of "registers.h".
	//
	// For the interrupts handled here (different from FIQ),
	// only 3 registers are banked: SP(r13), LR (r14), and
	// CPSR. All other registers (r0-r12) are those of the
	// interrupted process. Then, CPSR must be obtained
	// from SPSR, and SP and LR are banked, so that they
	// can only be obtained after switch to
	// system mode (not normal mode, because switch back would
	// be impossible).	
	//
	// First, save r0. I could do this at the same time with
	// r1-r12 without this if I used the stack. However,
	// I don't want to use the stack in this macro.
	str	r0, _r0_cs
	// Then, save registers r1-r12 using r0
	ldr	r0, =_r1_cs
	stmia	r0!, {r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}
	// Register lr contains now the pc of the interrupted
	// code, so I will place the value of lr into the pc of
	// the saved context. The actual lr will be recovered upon
	// entry in system mode.
	sub 	lr, lr, #\lroffset
	str	lr, _pc_cs
	// Save the SPSR, which is the user mode CPSR of
	// the interrupted process (using r0).
	// Note that I assume here that I'm coming from user
	// mode.
	mrs 	r0, spsr
	str	r0, _cpsr_cs	
	// To access LR and SP (and save them) I first need to
	// enter user or system mode (I choose to enter system
	// mode, because I may need to switch back, and this is
	// not possible from user mode).
	// IMPORTANT: this code works correctly only if I make
	// the hypothesis that all interruptible code (including
	// drivers) is executed in user or system mode. Otherwise,
	// I should take care about which banked registers I save.
	// This could be also be done without changing the mode
	// using direct access with the MRS instruction, e.g.:
	// mrs	r0, r13_usr // r13_usr is sp of user mode
	// str	r0, _sp_cs
	entermode CPSR_MODE_SYSTEM
	str	sp, _sp_cs
	str	lr, _lr_cs
	// Switch back to supervisor (SVR) mode, which is the mode
	// in which the scheduler is executed. In this mode, it
	// is possible to use of LDMIA with PC and CPSR, which cannot
	// be done in user or system modes.
 	// Attention, this means that I will need to change the
 	// mode when entering interruptible kernel code.
	// I do not simply switch the mode. I also change the
	// stack base to the kernel non-preemptible stack.
 	entermode CPSR_MODE_SVR
	mov sp, #KERNEL_STACK_BASE
.endm

