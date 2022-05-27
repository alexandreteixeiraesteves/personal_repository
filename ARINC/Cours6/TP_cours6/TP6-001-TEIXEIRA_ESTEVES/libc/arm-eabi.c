
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
#include <libc/stdint.h>   // For uint32_t and int32_t
#include <libc/math.h>     // For abs
#include <librpi/debug.h>  // For debug_puts
#include <libc/stdio.h>    // For unsigned2ascii and int2ascii
#include <libc/stdlib.h>   // For fatal_error
#include <libc/arm-eabi.h> 

/* This C file is particular. 
   These functions are part of the ARM ABI (Application 
   Binary Interface). They include, for instance, the 
   implementation of the integer division (as no operation
   exists in hardware). The compiler automatically transforms
   / on integers into calls to this function...

   I do **NOT** want these functions to be used.
   The reason is simple: it's a pain in the A**.
   The ARM EABI expects many of these functions to get 
   their arguments and return their results in registers,
   even when there are multiple return values.
   Doing this in GCC requires tweaking the 
   C calling interfaces by declaring assembly wrappers
   (I could not find a simpler way of doing it).

   For this reason, these functions are all simple
   failure routines meant to show to the user they
   need to use standard C calls to functions with 
   different names.
*/

// These functions do not need to be defined in assembly
// like __aeabi_uidivmod and __aeabi_idivmod because they 
// return scalar values (integers), not a struct.
uint32_t 
__aeabi_uidiv(uint32_t numerator,uint32_t denominator) {
  return rpi_uidiv(numerator,denominator) ;
}
int32_t 
__aeabi_idiv(int32_t numerator,int32_t denominator) {
  return rpi_idiv(numerator,denominator) ;
}


uidiv_return 
rpi_uidivmod(uint32_t numerator,uint32_t denominator) {
  // Implementation of a pseudo-code taken from Wikipedia
  // http://en.wikipedia.org/wiki/Division_algorithm
  /* Here's the pseudo-code:
     Inputs: N(numerator), D(denominator)
     Q := 0                 -- initialize quotient and remainder to zero
     R := 0                     
     for i = n-1...0 do     -- where n is number of bits in N
     R := R << 1          -- left-shift R by 1 bit
      R(0) := N(i)         -- set the least-significant bit of R 
                           --  equal to bit i of the numerator
      if R >= D then
        R = R - D
	Q(i) := 1
      end
     end
  */
  if(denominator==0) {
    fatal_error("rpi_uidivmod: Division by zero. Aborting execution...\n") ;
  }
  // For storing the result and the intermediate values.
  uidiv_return r ; 
  r.quot = 0 ;
  r.rem = 0 ;   
  // Iterate 32 times 
  uint32_t i ;
  for(i=0;i<32;i++) {
    uint32_t j = 31-i ; 
    r.rem = r.rem << 1 ;
    r.rem |= ((numerator & (1 << j))?1:0) ;
    if(r.rem>=denominator){
      r.rem -= denominator ;
      r.quot |= (1<<j) ;
    }
  }
  return r ;
}

uint32_t 
rpi_uidiv(uint32_t numerator,uint32_t denominator) {
  register uidiv_return r = rpi_uidivmod(numerator,denominator) ;
  return r.quot ;
}

uint32_t 
rpi_uimod(uint32_t numerator,uint32_t denominator) {
  register uidiv_return r = rpi_uidivmod(numerator,denominator) ;
  return r.rem ;
}

// Signed integer division is nastier than unsigned one.
// Unsigned integer division is the implementation
// of the euclidean division with a non-negative reminder 
// smaller than the absolute value of the denominator.
// In signed integer division the rule is that truncation 
// of the quotient is done towards 0, which for negative
// numbers means that the reminder can be negative.
//
// It's actually like this: To do the signed divmod,
// one needs to do the unsigned divmod of the absolute
// values when they are non-negative (attention, these 
// can be negative for MININT). Then, play with the 
// signs of the quotient and the reminder to match...
// cf. http://stackoverflow.com/questions/3602827/
//       what-is-the-behavior-of-integer-division-in-c
// TODO: test this code.
idiv_return 
rpi_idivmod(int32_t numerator,int32_t denominator) {
  uint32_t unum ;
  uint32_t uden ;
  {
    int32_t tmp = abs(numerator) ;
    // According to the standard, abs can return a negative
    // value if the argument is MININT = -MAXINT-1. 
    // Check if can't simply convert the result of abs.
    if(tmp<0) unum = ((uint32_t)MAXINT)+1 ;
    else unum = (uint32_t)tmp ;
    tmp = abs(denominator) ;
    if(tmp<0) uden = ((uint32_t)MAXINT)+1 ;
    else uden = (uint32_t)tmp ;
  }
  // I start by computing the unsigned quot and rem.
  uidiv_return ur = rpi_uidivmod(unum,uden) ;
  // In all cases, I start by placing in the result
  // the quot and rem computed from the absolute 
  // values. There should be no conversion problem
  // here because unum and uden are <= MAXINT+1, 
  // which implies quot, rem <= MAXINT
  idiv_return r = {(int32_t)ur.quot,(int32_t)ur.rem} ;
  // If the signs of the two are different, the 
  // quotient is negative or 0.
  if((r.quot != 0)&&
     ((numerator ^ denominator) & 0x8000000)) {
    r.quot = -r.quot ;
  }
  // If the numerator is negative, the reminder is
  // negative or 0.
  if((r.rem != 0)&&
     (numerator <0)) {
    r.rem = -r.rem ;
  }
  return r ;
}

int32_t 
rpi_idiv(int32_t numerator,int32_t denominator) {
  register idiv_return r = rpi_idivmod(numerator,denominator) ;
  return r.quot ;
}

int32_t 
rpi_imod(int32_t numerator,int32_t denominator) {
  register idiv_return r = rpi_idivmod(numerator,denominator) ;
  return r.rem ;
}

