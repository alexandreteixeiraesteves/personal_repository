
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
#include <librpi/debug.h>
#include <libpartition/svc-call.h>
#include <libpartition/svc-l2-request.h>

// Statically allocated L2Request structure
// that will be used by the calls.
struct L2Request l2_request ;

// Require that PERIODIC_WAIT is performed on the currently
// executing process of the partition.
uint32_t svc_l2_request_periodic_wait() {
  //debug_printf("svc_l2_request_periodic_wait:\n") ;
  l2_request.type = L2_REQ_PERIODIC_WAIT ;
  return svc_sched_l2_request(&l2_request) ;
}

/* Require that STOP is performed on a process. */
uint32_t svc_l2_request_stop(PROCESS_ID_TYPE target_pid,
			     PROCESS_ID_TYPE caller_pid) {
  //debug_printf("svc_l2_request_stop:\n") ;
  l2_request.type = L2_REQ_STOP ;
  l2_request.target_pid = target_pid ;
  l2_request.caller_pid = caller_pid ;
  return svc_sched_l2_request(&l2_request) ;
}

/* Require that DELAYED_START is performed on a process. */
uint32_t svc_l2_request_delayed_start(PROCESS_ID_TYPE target_pid,
				      PROCESS_ID_TYPE caller_pid,
				      SYSTEM_TIME_TYPE delay) {
  //debug_printf("svc_l2_request_delayed_start:\n") ;
  l2_request.type = L2_REQ_DELAYED_START ;
  l2_request.target_pid = target_pid ;
  l2_request.caller_pid = caller_pid ;
  l2_request.delay = delay ;
  return svc_sched_l2_request(&l2_request) ;
}

/* Require that SUSPEND or SUSPEND_SELF is performed on a process. */
uint32_t svc_l2_request_suspend(PROCESS_ID_TYPE target_pid,
				PROCESS_ID_TYPE caller_pid,
				SYSTEM_TIME_TYPE delay) {
  //debug_printf("svc_l2_request_suspend:\n") ;
  l2_request.type = L2_REQ_SUSPEND ;
  l2_request.target_pid = target_pid ;
  l2_request.caller_pid = caller_pid ;
  l2_request.delay = delay ;
  return svc_sched_l2_request(&l2_request) ;
}

/* Require that RESUME is performed on a process. */
uint32_t svc_l2_request_resume(PROCESS_ID_TYPE target_pid,
			       PROCESS_ID_TYPE caller_pid) {
  //debug_printf("svc_l2_request_resume:\n") ;
  l2_request.type = L2_REQ_RESUME ;
  l2_request.target_pid = target_pid ;
  l2_request.caller_pid = caller_pid ;
  return svc_sched_l2_request(&l2_request) ;
}

/* Require that TIMED_WAIT is performed by a process. */
uint32_t svc_l2_request_timed_wait(SYSTEM_TIME_TYPE delay) {
  //debug_printf("svc_l2_request_timed_wait:\n") ;
  l2_request.type = L2_REQ_TIMED_WAIT ;
  l2_request.target_pid = -1 ;
  l2_request.caller_pid = -1 ;
  l2_request.delay = delay ;
  return svc_sched_l2_request(&l2_request) ;
}
