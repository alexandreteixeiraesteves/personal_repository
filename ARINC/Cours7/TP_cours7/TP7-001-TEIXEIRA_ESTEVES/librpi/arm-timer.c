
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
#include <libc/stdint.h>        // For uint32_t
#include <librpi/mmap.h>        // For base addresses
#include <librpi/interrupts.h>  // For the ICU registers.

enum PreScaleValues {
  PRESCALE_NONE  = 0 ,
  PRESCALE_16    = 1 ,
  PRESCALE_256   = 2 ,
  PRESCALE_NONE1 = 3   // weird...
} ;

union ARMTimerControlRegister {
  struct {
    uint32_t Reserved : 1 ;
    uint32_t Mode23Bit : 1 ;
    enum PreScaleValues PreScale : 2 ;
    uint32_t Reserved2 : 1 ;
    uint32_t TimerInterruptEnable : 1 ;
    uint32_t Reserved3 : 1 ;
    uint32_t TimerEnabled : 1 ;
    uint32_t TimerHaltedInDebugMode : 1 ;
    uint32_t FreeRunningEnabled : 1 ;
    uint32_t Reserved4 : 6 ;
    uint32_t FreeRunningPreScale : 8 ;
    uint32_t Reserved5 : 8 ;
  } decoder ;
  uint32_t bv ;
} ;

struct ARMTimerRegisters {
  /* The timer load register sets the time for the timer to count
     down.  This value is loaded into the timer value register
     after the load register has been written or if the timer-value
     register has counted down to 0. */
  volatile uint32_t Load;
 
  /* This register holds the current timer value and is counted down when
     the counter is running. It is counted down each timer clock until the
     value 0 is reached. Then the value register is re-loaded from the
     timer load register and the interrupt pending bit is set. The timer
     count down speed is set by the timer pre-divide register. */
  volatile uint32_t Value;
 
  /* The standard SP804 timer control register consist of 8 bits but in the
     BCM implementation there are more control bits for the extra features.
     Control bits 0-7 are identical to the SP804 bits, albeit some
     functionality of the SP804 is not implemented. All new control bits
     start from bit 8 upwards. */
  volatile union ARMTimerControlRegister Control;
 
  /* The timer IRQ clear register is write only. When writing this register
     the interrupt-pending bit is cleared. When reading this register it
     returns 0x544D5241 which is the ASCII reversed value for "ARMT". */
  volatile uint32_t IRQClear;
 
  /* The raw IRQ register is a read-only register. It shows the status of
     the interrupt pending bit. 0 : The interrupt pending bits is clear.
     1 : The interrupt pending bit is set.
     
     The interrupt pending bits is set each time the value register is
     counted down to zero. The interrupt pending bit can not by itself
     generates interrupts. Interrupts can only be generated if the
     interrupt enable bit is set. */
  volatile uint32_t RAWIRQ;
 
  /* The masked IRQ register is a read-only register. It shows the status
     of the interrupt signal. It is simply a logical AND of the interrupt
     pending bit and the interrupt enable bit. 0 : Interrupt line not
     asserted. 1 :Interrupt line is asserted, (the interrupt pending and
     the interrupt enable bit are set.)  */
  volatile uint32_t MaskedIRQ;
 
  /* This register is a copy of the timer load register. The difference is
     that a write to this register does not trigger an immediate reload of
     the timer value register. Instead the timer load register value is
     only accessed if the value register has finished counting down to
     zero. */
  volatile uint32_t Reload;
  
  /* The Pre-divider register is not present in the SP804. The pre-divider
     register is 10 bits wide and can be written or read from. This
     register has been added as the SP804 expects a 1MHz clock which we do
     not have. Instead the pre-divider takes the APB clock and divides it
     down according to:
     
     timer_clock = apb_clock/(pre_divider+1)
     
     The reset value of this register is 0x7D so gives a divide by 126. */
  volatile uint32_t PreDivider;
 
  /* The free running counter is not present in the SP804. The free running
     counter is a 32 bits wide read only register. The register is enabled
     by setting bit 9 of the Timer control register. The free running
     counter is incremented immediately after it is enabled. The timer can
     not be reset but when enabled, will always increment and roll-over.
     
     The free running counter is also running from the APB clock and has
     its own clock pre-divider controlled by bits 16-23 of the timer
     control register.
     
     This register will be halted too if bit 8 of the control register is
     set and the ARM is in Debug Halt mode. */
  volatile uint32_t FreeRunningCounter; 
} ;

static struct ARMTimerRegisters* rpiARMTimerRegisters = 
  (struct ARMTimerRegisters*)ARMTIMER_BASE;

void ArmTimerStart() {
  /* Timer frequency = Clk/256 * 0x400 */
  rpiARMTimerRegisters->Load = 0x400;
  
  union ARMTimerControlRegister reg ;
  reg.bv = 0 ;
  reg.decoder.Mode23Bit = 1 ;
  reg.decoder.TimerEnabled = 1 ;
  reg.decoder.TimerInterruptEnable = 1 ;
  reg.decoder.FreeRunningPreScale = PRESCALE_256 ;
  /* Setup the ARM Timer */
  rpiARMTimerRegisters->Control = reg ;
  // Also, I need to enable the basic IRQ entry, but
  // nothing else.
  union BaseInterruptEnableRegister bier ;
  bier.bv = 0 ;
  bier.decoder.Timer = 1 ;
  rpiICURegisters->Enable_Basic_IRQs = bier ;
}

void ArmTimerRefresh() {
  /* Clear the ARM Timer interrupt - it's the only interrupt we have
     enabled, so we want don't have to work out which interrupt source
     caused us to interrupt */
  rpiARMTimerRegisters->IRQClear = 1 ;
}
