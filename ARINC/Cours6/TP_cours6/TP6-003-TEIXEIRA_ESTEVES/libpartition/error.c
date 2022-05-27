
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
#include <arinc653/types.h>      // For the ARINC 653 error codes
#include <kernel/errors.h>       // For the additional codes
                                 // (implem-dependent).
#include <libpartition/error.h>  
#include <libc/stdio.h>

char buf[64] ;

const char* error2string(uint32_t error_code) {
  switch(error_code){
    // codes from arinc653/types.h
  case NO_ERROR: return "NO_ERROR" ;
  case NO_ACTION: return "NO_ACTION" ;
  case NOT_AVAILABLE: return "NOT_AVAILABLE" ;
  case INVALID_PARAM: return "INVALID_PARAM" ;
  case INVALID_CONFIG: return "INVALID_CONFIG" ;
  case INVALID_MODE: return "INVALID_MODE" ;
  case TIMED_OUT: return "TIMED_OUT" ;
    // codes from kernel/errors.h
  case SVC_UNIMPLEMENTED: return "SVC_UNIMPLEMENTED" ;
  case SVC_BAD_CALL_ID: return "SVC_BAD_CALL_ID" ;
  case CONSOLE_OVERFLOW: return "CONSOLE_OVERFLOW" ;
  default: 
    snprintf(buf,64,"Unknown(%d)",error_code) ;
    return buf ;
  }
}

