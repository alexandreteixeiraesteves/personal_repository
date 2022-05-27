
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
#include <libc/stdio.h>
#include <librpi/registers.h>
#include <librpi/debug.h>

void restore_context_and_resume(const struct FullRegisterSet* const rs) {
  // TODO: this code is wrong, it misses the SPSR initialization.
  // But if I make it correct, I have to make sure it's never called
  // on kernel code.
  asm volatile(// Save CPSR to r0
	       "mov   r2, %[sp];"
	       "mov   r3, %[lr];"
	       "mov   r4, %[cpsr];"
	       "mov   r5, %[x];"
	       "mrs   r0, cpsr;"
	       // Enter SYSTEM mode, interrupts disabled
	       "mov   r1, #0xdf;"         
	       "msr   cpsr, r1;"
	       // Place the new sp and lr
	       "mov   sp, r2;"
	       "mov   lr, r3;"
	       // Return to the SVR mode.
	       "msr   cpsr, r0;"
	       // Make the rest of the context switch.
	       "msr   spsr, r4;"
	       "ldmia r5, {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}^"
	       :
	       :[x] "r" (rs),
		[cpsr] "r" (rs->cpsr),
		[sp] "r" (rs->sp),
		[lr] "r" (rs->lr)
	       : "r0", "r1", "r2", "r3", "r4", "r5", "memory") ;
}

void PrintRegisterSet(const struct FullRegisterSet* const rs) {
  debug_printf("r0:%x r1:%x r2:%x  r3:%x  r4:%x  r5:%x r6:%x\n"
	       "r7:%x r8:%x r9:%x r10:%x r11:%x r12:%x\n"
	       "sp:%x lr:%x pc:%x cpsr:%x\n",
	       rs->r[0],rs->r[1],rs->r[2],rs->r[3],
	       rs->r[4],rs->r[5],rs->r[6],
	       rs->r[7],rs->r[8],rs->r[9],rs->r[10],
	       rs->r[11],rs->r[12],
	       rs->sp,rs->lr,rs->pc,rs->cpsr) ;
}

uint32_t get_stack_pointer(){
  register uint32_t SP asm ("sp") ;
  return SP ;
}

void enable_interrupts() {
  register CPSR_type R0 asm ("r0") ;
  // Read CPSR
  asm volatile("mrs     r0, cpsr;":::"r0") ;
  // Unset the bit disabling IRQs, cf. ARM ARM, A1.1.3, A2.5
  R0.decoder.I = 0 ;
  // Set the new CPSR
  asm volatile("msr     cpsr_c, r0":::"r0") ;
}

void disable_interrupts() {
  register CPSR_type R0 asm ("r0") ;
  // Read CPSR
  asm volatile("mrs     r0, cpsr;":::"r0") ;
  // Set the bit disabling IRQs, cf. ARM ARM, A1.1.3, A2.5
  R0.decoder.I = 1 ;
  // Set the new CPSR
  asm volatile("msr     cpsr_c, r0":::"r0") ;
}

