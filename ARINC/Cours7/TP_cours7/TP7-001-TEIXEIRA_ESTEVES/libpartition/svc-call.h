
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
#ifndef SVC_CALL_H
#define SVC_CALL_H

#include <libc/stdint.h>                 // For uint32_t
#include <librpi/registers.h>            // For FullRegisterSet
#include <arinc653/partition.h>          // For OPERATING_MODE_TYPE
#include <libpartition/svc-l2-request.h> // For L2Request
#include <kernel/queuing.h>              // For the queuing data structure.

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
uint32_t svc_sched_restore_process_context(struct FullRegisterSet*context) ;

// Request a partition mode change.
// The call may or may not return. For the cases where it
// returns, there is a return (error) code.
uint32_t svc_sched_set_partition_mode(OPERATING_MODE_TYPE new_mode) ;

// A process requests a service of the L2 scheduler.
// Context will be given back, if necessary, through
// svc_restore_process_context, with the result in the
// struct passed in argument.
uint32_t svc_sched_l2_request(struct L2Request* req) ;

uint32_t svc_send_queuing(uint32_t port_id,
			  struct QueuingMessage* message) ;
uint32_t svc_recv_queuing(uint32_t port_id,
			  struct QueuingMessage* message) ;


/*----------------------------------------------------------*/
/* ARINC 653 services that don't change the scheduler       */
/* state.                                                   */
/*----------------------------------------------------------*/
// Get time. Output is in the result parameter.
uint32_t svc_a653_get_time(SYSTEM_TIME_TYPE* result) ;

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
uint32_t svc_puts(const char *str) ;

// Print one line of text on the screen.
// It's done directly by the L1 kernel (not by the system
// partition) in order to remain non-interruptible and
// immediate, thus easily traceable.
uint32_t svc_print_line_atomic(const PROCESS_ID_TYPE pid,
			       const char *str) ;

// 
uint32_t svc_read_level(uint32_t*p_lvl) ;

#endif
