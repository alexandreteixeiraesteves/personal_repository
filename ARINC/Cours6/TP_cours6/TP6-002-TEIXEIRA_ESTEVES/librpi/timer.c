
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
#include <librpi/mmap.h>     // For SYSTIMER_BASE
#include <librpi/mmio.h>     // For MMIO_READ and mmio_read
#include <librpi/debug.h>    // For debug_printf
#include <librpi/timer.h>

//#define timer_debug_printf(...) debug_printf(__VA_ARGS__)
#define timer_debug_printf(...)


typedef struct {
   uint32_t control_status;
   uint32_t counter_lo;
   uint32_t counter_hi;
   uint32_t compare0;
   uint32_t compare1;
   uint32_t compare2;
   uint32_t compare3;
} rpi_sys_timer_t;

static  rpi_sys_timer_t* rpiSystemTimer = 
  (rpi_sys_timer_t*)SYSTIMER_BASE;

/*  Sleep with timeout function.
 *  It takes as input a duration in microseconds (us),
 *  an address to read with mmio_read,
 *  a mask to apply on the value that was read,
 *  and a test type.
 *  If the address is 0, then no test is made and the 
 *  function is a pure timeout.
 *  If the address is different from 0:
 *  - if the test_value is 0, then the timeout
 *    occurs when mmio_read(address)&mask == 0.
 *  - if test_value!=0, then the timeout occurs
 *    when mmio_read(address)&mask != 0.
 *  Otherwise, no timeout occurs. 
 *
 *  The address can be that of an MMIO (it follows the
 *  correct protocol).
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
			int debug_print) {
  // Attention, the order of the following two declarations
  // is important, because reading counter_hi should happen
  // before reading counter_lo (otherwise, there is a small 
  // probability of having the rollover between reading
  // lo and reading hi, which leads to a huge timeout.
  volatile uint32_t hi = MMIO_READ(rpiSystemTimer->counter_hi);
  volatile uint32_t lo = MMIO_READ(rpiSystemTimer->counter_lo);
  
  if(debug_print) {
    timer_debug_printf("usleep_with_timeout:\n") ;
    timer_debug_printf("\t us=%x hi=%x lo=%x [addr]=%x mask=%x \n",
		       us,hi,lo,
		       mmio_read(address),
		       mask) ;
  }
  
  if(us>=(0xffffffff-lo)){
    if(debug_print){
      timer_debug_printf("\t roll over needed.\n") ;
    }
    // The counter_lo counter will roll over before
    // reaching the needed timeout. To test rollover
    // I just test the change in counter_hi.
    while(MMIO_READ(rpiSystemTimer->counter_hi) == hi) {
      if(address) {
	if(mmio_read(address)&mask) {
	  if(test_value) return 1;
	} else {
	  if(!test_value) return 1;
	}
      }
    }
    // Rollover done, I update the values I need to test
    // next.
    us -= 0xffffffff-lo ;
    lo = 0 ;
  }
  // Now, I just have to wait until the lo bits are 
  // high enough.
  while((MMIO_READ(rpiSystemTimer->counter_lo) - lo) < us) {
    if(address) {
      if(mmio_read(address)&mask) {
	if(test_value)return 1;
      } else {
	if(!test_value)return 1;
      }
    }
  }
  if(debug_print){
    timer_debug_printf("\t no timeout occurred.\n") ;
  }
  return 0 ;
}

int usleep( uint32_t us ) {
  return usleep_with_timeout(us,0,0,0,0) ;
}
