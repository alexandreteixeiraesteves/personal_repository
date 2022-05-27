
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
#ifndef SVC_H
#define SVC_H

#include <arinc653/types.h> // For the APEX error codes.

//------------------------------------------------------------
// The SVC call protocol is here as follows:
// - The caller sets:
//   * the code in the statement and r0 to be equal to
//     the svn call identifier
//   * r1 and r2 are potential parameters of the call
// - Once the call is finished, if the call returns to
//   caller:
//   * r0 will contain the return value.
//   * r4-r12, lr and the cpsr are restored (for cpsr it is
//     implicit?)
//   
// The call is executed in the context (stack, registers)
// of the caller. Registers r4-r12 and lr are saved and
// restored if
//------------------------------------------------------------

enum SVC_call_identifiers {
  // Scheduler operations that may result in a state change.
  SVC_sched_restore_process_context = 0x10, // L2 scheduler cannot do it directly.
                                            // Context address is provided in r0.
                                            // The restored context is always a
                                            // process.
  SVC_sched_l2_request              = 0x11, // A process requires a scheduling service
                                            // from the L2 scheduler.
  SVC_sched_set_partition_mode      = 0x12, //
  SVC_send_queuing                  = 0x20,
  SVC_recv_queuing                  = 0x21,
  
  // ARINC 653 services that don't change the scheduler state.
  SVC_a653_get_time                 = 0x100, // Get system time in nanoseconds,
                                            // in r0.

  // Implementation-specific services
  SVC_puts                          = 0x200, // The string address is provided
                                            // in r1.
  SVC_print_line_immediate          = 0x201, // The string address is provided
                                            // in r1.
  SVC_read_level                    = 0x301, // The return value is provided in
                                             // r0, like for get_time.
} ;

#endif
