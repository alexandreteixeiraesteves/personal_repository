
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
#include <libc/stdio.h>                // Debug: unsigned2ascii (which
                                       // works in very poor contexts).
#include <libc/stddef.h>               // For UNUSED.
#include <kernel/boot-aux.h>           // For _context_save_struct
#include <kernel/scheduler.h>          // For L1_scheduler
#include <kernel/system-partition-console.h>  // For the console driver
#include <kernel/svc.h>                // For the supervisory call ids.
#include <kernel/errors.h>             // Implem-dependent errors.
#include <librpi/indent.h>             //
#include <kernel/kernel-debug.h>       //
#include <kernel/queuing.h>            //

const char* svc_call_id2string(enum SVC_call_identifiers id) {
  switch(id) {
  case SVC_sched_restore_process_context: return "restore context" ;
  case SVC_sched_l2_request: return "l2 request" ;
  case SVC_sched_set_partition_mode: return "set part mode" ;
  case SVC_recv_queuing: return "recv_queuing";
  case SVC_send_queuing: return "send_queuing";
  case SVC_a653_get_time: return "get time" ;
  case SVC_puts: return "console puts" ;
  case SVC_print_line_immediate: return "debug print line" ;
  default: return "**ERROR**" ;
  }
}

//---------------------------------------------------------
// SWI handler.
// To avoid all function call protocol interference with
// input parameters, this function takes none (registers
// r0-r2 are assumed unknown). But of course they are
// saved in the register context.
//
// SWIs should be the only means for the partitions to
// access peripherals (through drivers) and to do 
// inter-partition communications.
//
// Like all handlers, this should be kept short (in time).
// To do so, calls that take time should be relegated
// to a special "partition" which contains the drivers.
// For instance, printing on the UART should be done 
// by enqueuing the calls.
//---------------------------------------------------------
uint32_t svc_handler(uint32_t svc_id, uint32_t r1, uint32_t r2) {
  indent_set_depth(1) ;
  /* debug_printf(">> SVC(%s) r1=0x%x r2=0x%x\n", */
  /* 	       svc_call_id2string(svc_id),r1,r2) ; */
  /* if(svc_id != SVC_print_line_immediate) { */
  /*   indent_increment_depth() ; */
  /* } */
  
  switch(svc_id) {
  case SVC_sched_restore_process_context:
    // Restore the desired context.
    L1_restore_process_context(&_context_save_struct,
			       (struct FullRegisterSet*)r1) ;
    
  case SVC_sched_l2_request:
    // This one does not return. Instead, it calls the
    // L2 scheduler.
    L1_scheduler(PARTITION_L2_EVENT,(void*)r1) ;
    
  case SVC_sched_set_partition_mode:
    // This call may or may not return (with an error code)
    // depending on whether the requested change is legal.
    return L1_set_partition_mode(r1) ;

  case SVC_send_queuing:
    // Send a queuing message, if there is place in the
    // queue (non-blocking).
    return queuing_send(l1_scheduler_state.active_partition,
			r1,
			(struct QueuingMessage*)r2) ;
    
  case SVC_recv_queuing:
    // Receive a queuing message, if there is one in the
    // queue (non-blocking).
    return queuing_recv(l1_scheduler_state.active_partition,
			r1,
			(struct QueuingMessage*)r2) ;
    
  case SVC_a653_get_time:
    {
      SYSTEM_TIME_TYPE *p_res = (SYSTEM_TIME_TYPE*)r1 ;
      *p_res = L1_get_time() ;
    }
    // There can be no error here.
    return NO_ERROR ;
    
  case SVC_puts:
    // Enqueuing for printing by driver in system partition.
    // Return the value.
    return console_log_message((char*)r1) ;

  case SVC_print_line_immediate:
    // Debug printing.
    debug_print_line_partition(l1_scheduler_state.active_partition,
			       (char*)r1,
			       r2) ;
    // No error possible.
    return NO_ERROR ;

  case SVC_read_level:
    {
      uint32_t *p_res = (uint32_t*)r1 ;
      *p_res = console_read_level() ;
    }
    return NO_ERROR ;
    
  default:
    // Debug printing.
    debug_puts("Unhandled SVC id. Saved register set:\n") ;
    indent_increment_depth() ;
    PrintRegisterSet(&_context_save_struct) ;
    indent_decrement_depth() ;
    debug_puts("Returning:\n") ;
    // Return with error. This value is placed in r0 under ARM EABI.
    return SVC_BAD_CALL_ID ;
  }
}
