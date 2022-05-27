
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
#include <librpi/debug.h>              // For debug_printf, debug_puts
#include <librpi/indent.h>             //
#include <libc/stddef.h>               // For NULL
#include <librpi/system_timer.h>       // For SystemTimerRefresh
#include <kernel/scheduler.h>          // For L1_scheduler
#include <libc/stdio.h>                // For unsigned2ascii

//---------------------------------------------------------
// IRQ handler that triggers the L1 scheduler routine.
// I should extend this handler to decode the interrupt
// request vector.
//---------------------------------------------------------
void irq_handler() {  
  static uint32_t tick_counter = 0 ;
  //  static char print_buf[32];
  indent_set_depth(0) ; // Reset the printing depth to 0
  //uint32_t mtf_cnt = tick_counter/l1_scheduler_state.mtf;
  //uint32_t mtf_off = tick_counter%l1_scheduler_state.mtf;
  //debug_printf("\nIRQ:TICK(%d:%d):\n",mtf_cnt,mtf_off) ;
  //  uint32ascii(tick_counter,16,8,10,print_buf) ;
  //debug_puts(print_buf);
  //debug_puts("\n") ;
  tick_counter++ ;
  // The all-important clearing of interrupts.
  // One does not return from this sort of interrupt.
  SystemTimerRefresh() ;
  L1_scheduler(KERNEL_TICK_EVENT, NULL) ;
}
