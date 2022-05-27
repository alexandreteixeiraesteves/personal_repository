
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
#include <libc/stdio.h>
#include <libpartition/svc-call.h> // For the SVC call
#include <librpi/debug.h>
#include <librpi/indent.h>
#include <libpartition/scheduler.h>

// TODO: like for the kernel, add the indentation, so that
// status printing goes smoothly. In conjunction with light
// modifications in the L1 kernel, this should allow
// identifying who is printing (L2 kernel vs. task id).

void debug_print_line(const unsigned char local_indent_depth,
		      const PROCESS_ID_TYPE running_process_id,
		      const char* str) {
  char buf[256] ;
  int i ;
  for(i=0;i<local_indent_depth;i++) buf[i]='\t';
  snprintf(buf+local_indent_depth,255-local_indent_depth,
	   "%s",
	   str) ;
  svc_print_line_atomic(running_process_id,
			buf) ;
}


// This is needed in librpi/debug.h
void debug_print_line_atomic(const char* str) {
  debug_print_line(indent_get_depth(),
		   l2_scheduler_state.running_process,
		   str) ;
}
