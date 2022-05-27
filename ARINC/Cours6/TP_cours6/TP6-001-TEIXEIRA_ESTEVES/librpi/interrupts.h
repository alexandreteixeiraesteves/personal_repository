
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
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

// IRQ_basic_pending decoding, partial (the other  can
// be found
union BaseInterruptEnableRegister {
  struct {
    uint32_t Timer : 1 ;
    uint32_t Mailbox : 1 ;
    uint32_t Doorbell0 : 1 ;
    uint32_t Doorbell1 : 1 ;
    uint32_t GPU0 : 1 ;
    uint32_t GPU1 : 1 ;
    uint32_t Error1 : 1 ;
    uint32_t Error0 : 1 ;
    uint32_t Reserved : 24 ;
  } decoder ;
  uint32_t bv ;
} ;
  
// Interrupt control unit (ICU) registers.
struct ICURegisters {
  volatile uint32_t IRQ_basic_pending;
  volatile uint32_t IRQ_pending_1;
  volatile uint32_t IRQ_pending_2;
  volatile uint32_t FIQ_control;
  volatile uint32_t Enable_IRQs_1;
  volatile uint32_t Enable_IRQs_2;
  volatile union BaseInterruptEnableRegister Enable_Basic_IRQs;
  volatile uint32_t Disable_IRQs_1;
  volatile uint32_t Disable_IRQs_2;
  volatile uint32_t Disable_Basic_IRQs;
} ;

// Memory-mapped control registers at the good address.
extern struct ICURegisters* rpiICURegisters ;


#endif
