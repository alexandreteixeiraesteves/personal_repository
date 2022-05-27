
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
#include <libc/stdint.h>       // Basic types
#include <libc/stddef.h>       // For UNDEF
#include <librpi/registers.h>  // For struct FullRegisterSet
#include <librpi/debug.h>      // Explicit...
#include <kernel/boot-aux.h>   //


// The following struct is defined in assembly in file
// boot.S. It is used to reload the context of a
// task.
extern struct FullRegisterSet _context_load_struct ;

// Function defined in boot.S which loads the context
// of _context_load_struct.
extern void _context_load_asm() ;

// Load the context _context_load_struct (thus giving
// control to the context).
void restore_full_context(struct FullRegisterSet* regs) {
  // Full copy
  // debug_puts("restore_full_context with:\n") ;
  // PrintRegisterSet(regs) ;
  _context_load_struct = *regs ;
  _context_load_asm() ;
}

