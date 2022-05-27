
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
#ifndef MMIO_H
#define MMIO_H

/* mmio.h - access to memory-mapped IO (MMIO) registers */
/* Attention: some problems due to other causes may look
   like memory barrier problems. For instance, incorrect
   access to UART0 may lead to deadlocks, as detailed in
   uart.c. */

#include <libc/stdint.h> //For uint32_t

// Memory barrier
/* A memory barrier should do 2 things:
   - force the CPU operation ordering to respect
     data dependencies.
   - force the compiler to preserve it.
   On an in-order commit CPU like this one, the 
   CPU is rather nice, so I don't really need
   the full force of a memory barrier. However,
   it may be needed at some point in the future
   when communicating with the GPU.

   In the compiler, the problems come from 
   optimizations, which may reorder memory 
   accesses, or even remove them. This is 
   why some data must be labelled
   "volatile" and the sw memory barrier must
   be used.

   My memory barrier does both by combining
   the assembly code that enforces the barrier
   in HW with the inline assembly "memory" 
   which forces all memory accesses to complete
   before proceeding.
*/
__attribute__((always_inline))
inline void memory_barrier(void) {
  asm volatile ("mov	r0, #0;"
  		"mcr	p15, #0, r0, c7, c10, #5"
  		: : : "memory", "r0");
}

/* Write one value to an MMIO. */
__attribute__((always_inline))
inline void mmio_write(volatile uint32_t* reg_addr, uint32_t data) {
  memory_barrier() ;
  *reg_addr = data ;
  memory_barrier() ;
}
// And a macrodefinition to facilitate program writing.
// Now, writing to a uint32_t register is simply done as
// MMIO_WRITE(register_addr,value). This is even better 
// than using simple assignments, because it identifies 
// the MMIO accesses.
#define MMIO_WRITE(uint32_t_ptr,value) \
  mmio_write(&(uint32_t_ptr),value)


/* Read one value from an MMIO. */
__attribute__((always_inline))
inline uint32_t mmio_read(volatile uint32_t* reg_addr) {
  memory_barrier() ;
  register volatile uint32_t tmp = *reg_addr ;
  memory_barrier() ;
  return tmp ;
}
// And a macrodefinition to facilitate program writing.
#define MMIO_READ(uint32_t_ptr) (mmio_read(&(uint32_t_ptr)))

#endif
