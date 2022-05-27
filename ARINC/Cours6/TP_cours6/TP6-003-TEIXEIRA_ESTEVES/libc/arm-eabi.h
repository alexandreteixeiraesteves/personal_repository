
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
#ifndef ARM_EABI_H
#define ARM_EABI_H

#include <libc/stdint.h>

typedef struct { 
  uint32_t quot ;
  uint32_t rem ;
} uidiv_return ;

uidiv_return 
rpi_uidivmod(uint32_t numerator,uint32_t denominator) ;

uint32_t
rpi_uidiv(uint32_t numerator,uint32_t denominator) ;

uint32_t
rpi_uimod(uint32_t numerator,uint32_t denominator) ;

typedef struct { 
  int32_t quot ;
  int32_t rem ;
} idiv_return ;

idiv_return 
rpi_idivmod(int32_t numerator,int32_t denominator) ;

int32_t
rpi_idiv(int32_t numerator,int32_t denominator) ;

int32_t
rpi_imod(int32_t numerator,int32_t denominator) ;


#endif
