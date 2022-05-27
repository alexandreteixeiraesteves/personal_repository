
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
#ifndef ERRORS_H
#define ERRORS_H

// This file contains the definition of implementation-dependent
// error codes, not included in the original ARINC 653 error set.
// The two error sets can be used at the same time, as their
// values are exclusive (and also their definition covers different
// issues). More precisely, the APEX codes start at 0 (which is
// the NO_ERROR code). The implementation-specific errors have
// 0xf on the 4 most significant bits. Both APEX errors and these
// ones are transmitted over uint32_t return types.
enum ImplementationSpecificErrors {
  // SVC call errors
  SVC_UNIMPLEMENTED   = 0xF0000000, // Known, but unimplemented call id.
  SVC_BAD_CALL_ID     = 0xF0000001, // Not a valid SVC call id.
  // Console driver errors
  CONSOLE_OVERFLOW    = 0xF0000010, // The console buffer is full, cannot log.
} ;

#endif
