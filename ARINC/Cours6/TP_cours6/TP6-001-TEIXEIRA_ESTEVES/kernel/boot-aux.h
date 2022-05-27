
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
#include <librpi/registers.h>  // For struct FullRegisterSet

// The following struct is defined in assembly in file
// boot.S. It is used to save the context of a
// task. It is filled in by interrupts saving contexts
// (e.g. by the IRQ vector).
extern struct FullRegisterSet _context_save_struct ;

// Load the context _context_load_struct (thus giving
// control to the context).
void restore_full_context(struct FullRegisterSet* regs) ;
