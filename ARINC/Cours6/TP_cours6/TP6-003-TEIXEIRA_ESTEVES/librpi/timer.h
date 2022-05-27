
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
#ifndef TIMER_H
#define TIMER_H

#include <libc/stdint.h>

int usleep( uint32_t us ) ;

/*  Complex sleep with timeout function.
 *  It takes as input a duration in microseconds (us),
 *  an address to read with mmio_read,
 *  a mask to apply on the value that was read,
 *  and a test type.
 *  If the pointer ptr is 0, then no test is made and the 
 *  function is a pure timeout.
 *  If the pointer is different from 0:
 *  - if the test_value is 0, then the timeout
 *    occurs when mmio_read(address)&mask == 0.
 *  - if test_value!=0, then the timeout occurs
 *    when mmio_read(address)&mask != 0.
 *  Otherwise, no timeout occurs. 
 * 
 *  The function returns 1 in case timeout occurs,
 *  0 otherwise.
 *
 *  Attention: What this function guarantees is not an
 *  exact timout, but one that is greater than us and 
 *  smaller than us plus some reasonable precision.
 */
int usleep_with_timeout(uint32_t us, 
			volatile uint32_t* address,
			uint32_t mask,
			int test_value,
			int debug_print) ;

#endif
