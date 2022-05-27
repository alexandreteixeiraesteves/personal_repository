
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
#ifndef SVC_L2_REQUEST_H
#define SVC_L2_REQUEST_H

#include <arinc653/process.h>

/*----------------------------------------------------------*/
/* Process requests that go through SVC to the L2 scheduler,*/
/* thus ensuring the atomicity and consistency of the state.*/
/*----------------------------------------------------------*/

enum L2RequestType {
  L2_REQ_UNUSED                = 0,
  L2_REQ_PERIODIC_WAIT         = 1,
  L2_REQ_STOP                  = 2,
  L2_REQ_DELAYED_START         = 3,
  L2_REQ_SUSPEND               = 4,
  L2_REQ_RESUME                = 5,
  L2_REQ_TIMED_WAIT            = 6,
} ;

struct L2Request {
  enum L2RequestType type ;
  PROCESS_ID_TYPE    target_pid ;
  PROCESS_ID_TYPE    caller_pid ;
  SYSTEM_TIME_TYPE   delay ;
} ;


/* Require that PERIODIC_WAIT is performed on the currently
   executing process of the partition. */
uint32_t svc_l2_request_periodic_wait(void) ;

/* Require that STOP is performed on a process. */
uint32_t svc_l2_request_stop(PROCESS_ID_TYPE target_pid,
			     PROCESS_ID_TYPE caller_pid) ;

/* Require that DELAYED_START is performed on a process. */
uint32_t svc_l2_request_delayed_start(PROCESS_ID_TYPE target_pid,
				      PROCESS_ID_TYPE caller_pid,
				      SYSTEM_TIME_TYPE delay) ;

/* Require that SUSPEND or SUSPEND_SELF is performed on a process. */
uint32_t svc_l2_request_suspend(PROCESS_ID_TYPE target_pid,
				PROCESS_ID_TYPE caller_pid,
				SYSTEM_TIME_TYPE delay) ;

/* Require that RESUME is performed on a process. */
uint32_t svc_l2_request_resume(PROCESS_ID_TYPE target_pid,
			       PROCESS_ID_TYPE caller_pid) ;

/* Require that TIMED_WAIT is performed on a process. */
uint32_t svc_l2_request_timed_wait(SYSTEM_TIME_TYPE delay) ;

#endif
