
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
#ifndef KERNEL_DEBUG_H
#define KERNEL_DEBUG_H

#include <libc/stdint.h>

//
extern void debug_print_line_system_partition(const char* str) ;
//
extern void debug_print_line_partition(uint32_t partition_id,
				       const char* str,
				       int32_t process_id) ;

#endif
