
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
.globl __aeabi_uidivmod
.globl __aeabi_idivmod

// Assembly wrappers for the __aeabi_uidivmod
// and __aeabi_idivmod functions, to attain
// compliance with the ARM EABI,
// cf. IHI0043D_rtabi.pdf, page 20.
//
__aeabi_uidivmod:	
	// Save the return address
	push    {lr}
	// Make place for 3 words on the stack
	sub     sp, sp, #8
	// Save the 2 parameters 
	mov     r3, r0
	mov     r2, r1
	// Set the parameters for the rpi_uidivmod
	// function in the registers r0, r1, r2.
	// In r0 I place a pointer to the structure
	// where I will place the result.
	// In r1 and r2 the parameters, in order
	// (note that r2 is already in place).
	mov     r0, sp
	mov     r1, r3
	// Call the rpi_uidivmod function.
	bl      rpi_uidivmod
	// Now, get the results in r0 and r1 (the order
	// is crucial.
	pop	{r0,r1}
	// Return from the function
	pop     {pc}


// Assembly wrappers for the __aeabi_uidivmod
// and __aeabi_idivmod functions, to attain
// compliance with the ARM EABI,
// cf. IHI0043D_rtabi.pdf, page 20.
//
__aeabi_idivmod:	
	// Save the return address
	push    {lr}
	// Make place for 3 words on the stack
	sub     sp, sp, #8
	// Save the 2 parameters 
	mov     r3, r0
	mov     r2, r1
	// Set the parameters for the rpi_uidivmod
	// function in the registers r0, r1, r2.
	// In r0 I place a pointer to the structure
	// where I will place the result.
	// In r1 and r2 the parameters, in order
	// (note that r2 is already in place).
	mov     r0, sp
	mov     r1, r3
	// Call the rpi_idivmod function.
	bl      rpi_idivmod
	// Now, get the results in r0 and r1 (the order
	// is crucial.
	pop	{r0,r1}
	// Return from the function
	pop     {pc}
