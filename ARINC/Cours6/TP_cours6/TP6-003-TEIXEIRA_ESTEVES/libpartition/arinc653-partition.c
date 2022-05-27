
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
#include <libpartition/scheduler.h> // For l1_partition_interface
#include <libpartition/svc-call.h>  // For SVC calls
#include <arinc653/partition.h>


void GET_PARTITION_STATUS (/*out*/ PARTITION_STATUS_TYPE *PARTITION_STATUS,
			   /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  *PARTITION_STATUS = l1_partition_interface.partition_status ;
  *RETURN_CODE = NO_ERROR ;
}

void SET_PARTITION_MODE (/*in */ OPERATING_MODE_TYPE OPERATING_MODE,
			 /*out*/ RETURN_CODE_TYPE *RETURN_CODE ) {
  *RETURN_CODE = svc_sched_set_partition_mode(OPERATING_MODE) ;
}
