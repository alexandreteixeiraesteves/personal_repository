
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
#include <librpi/debug.h>          // For debug printing stuff
#include <kernel/svc.h>            // For the svc codes.
#include <libpartition/svc-call.h> // For svc-related defs.

/*
// Make an SVC call.
// Put the call id in r0, and possibly two
// parameters respectively in r1 and r2. Return a single
// value (the error code).
// The call id must be a constant, otherwise it cannot be
// put in the opcode. This is the very reason I use a
// #define here, and not an inline call (to be able to
// include the call id in the opcode).
#define make_svc_call(out_result,in_svc_id,in_r1,in_r2) \
  {                                              \
    debug_puts("Enter make_svc_call.\n") ;       \
    asm volatile("mov   r0, %[svc_id];"          \
		 "mov   r1, %[r1];"              \
		 "mov   r2, %[r2];"              \
		 "svc   %[svc_id];"              \
		 "mov   %[res], r0"              \
		 : [res]"=r"(out_result)  	 \
		 : [r1] "r" (in_r1),	         \
		   [r2] "r" (in_r2),		 \
		   [svc_id] "I" (in_svc_id)	 \
		 : "r0", "r1", "r2", "memory") ; \
    debug_puts("Exit make_svc_call.\n") ;	 \
  } \

*/

__attribute__((always_inline))
inline uint32_t make_svc_call(uint32_t svc_id,
			      uint32_t r1,
			      uint32_t r2) {
  uint32_t result ;
  //debug_printf("Enter make_svc_call with svc_id=0x%x.\n",
  //	       svc_id) ;
  asm volatile("mov   r0, %[sid];"
	       "mov   r1, %[xr1];"
	       "mov   r2, %[xr2];"
	       "svc   #0xababab;"
	       "mov   %[res], r0"
	       : [res]"=r" (result)
	       : [xr1] "r" (r1),
		 [xr2] "r" (r2),
		 [sid] "r" (svc_id)
	       : "r0", "r1", "r2", "memory") ;
  //debug_puts("Exit make_svc_call.\n") ;
  return result ;
}

  
    
/*----------------------------------------------------------*/
/* Scheduler operations that may result in a state change.  */
/*----------------------------------------------------------*/
  
// L2 scheduler restores the context of a process. 
// It cannot do it alone, because it does not have 
// supervisor rights (it runs in user mode). Thus, it must
// request it from the L1 kernel.
// If correctly formulated, this call does *not* return.
// However, a return value is produced, for the cases where
// the context change is refused.
uint32_t svc_sched_restore_process_context(struct FullRegisterSet*context) {
  /* uint32_t result ; */
  /* make_svc_call(result, */
  /* 		SVC_sched_restore_process_context, */
  /* 		(uint32_t)context, */
  /* 		0) ; */
  /* return result ; */
  return make_svc_call(SVC_sched_restore_process_context,
		       (uint32_t)context,
		       0) ;
}

// Request a partition mode change.
// The call may or may not return. For the cases where it
// returns, there is a return (error) code.
uint32_t svc_sched_set_partition_mode(OPERATING_MODE_TYPE new_mode) {
  /* uint32_t result ; */
  /* make_svc_call(result, */
  /* 		SVC_sched_set_partition_mode, */
  /* 		(uint32_t)new_mode, */
  /* 		0) ; */
  /* return result ; */
  return make_svc_call(SVC_sched_set_partition_mode,
		       (uint32_t)new_mode,
		       0) ;
}

// A process requests a service of the L2 scheduler.
// Context will be given back, if necessary, through
// svc_restore_process_context, with the result in the
// struct passed in argument.
uint32_t svc_sched_l2_request(struct L2Request* req) {
  return make_svc_call(SVC_sched_l2_request,
		       (uint32_t)req,
		       0) ;
}


/*----------------------------------------------------------*/
/* ARINC 653 services that don't change the scheduler       */
/* state.                                                   */
/*----------------------------------------------------------*/
// Get time. Output is in the result parameter.
uint32_t svc_a653_get_time(SYSTEM_TIME_TYPE* p_time) {
  /* uint32_t result ; */
  /* make_svc_call(result, */
  /* 		SVC_a653_get_time, */
  /* 		(uint32_t)p_time, */
  /* 		0) ; */
  /* return result ;   */
  return make_svc_call(SVC_a653_get_time,
		       (uint32_t)p_time,
		       0) ;
}

/*----------------------------------------------------------*/
/* Implementation-specific services.                        */
/*----------------------------------------------------------*/
// Send a null-terminated string to be printed by the
// kernel driver. To remain safe, the printing routine in
// the system partition may limit what is printed. For
// instance, it may accept max 256 characters and only
// ASCII characters. The function returns, but the
// error code only concerns the transfer to the system
// partition, not the execution of printf by the system
// partition.
uint32_t svc_puts(const char *str) {
  /* uint32_t result ; */
  /* make_svc_call(result, */
  /* 		SVC_puts, */
  /* 		(uint32_t)str, */
  /* 		0) ; */
  /* return result ;   */
  return make_svc_call(SVC_puts,
		       (uint32_t)str,
		       0) ;
}

// Print one line of text on the screen.
// It's done directly by the L1 kernel (not by the system
// partition) in order to remain non-interruptible and
// immediate, thus easily traceable.
uint32_t svc_print_line_atomic(const PROCESS_ID_TYPE pid,
			       const char *str) {
  return make_svc_call(SVC_print_line_immediate,
		       (uint32_t)str,
		       pid) ;
}

//
uint32_t svc_send_queuing(uint32_t port_id,
			  struct QueuingMessage* message) {
  return make_svc_call(SVC_send_queuing,
		       port_id,
		       (uint32_t)message) ;
}

//
uint32_t svc_recv_queuing(uint32_t port_id,
			  struct QueuingMessage* message) {
  return make_svc_call(SVC_recv_queuing,
		       port_id,
		       (uint32_t)message) ;
}


uint32_t svc_read_level(uint32_t* p_lvl) {
  return make_svc_call(SVC_read_level,
		       (uint32_t)p_lvl,
		       0) ;
}
