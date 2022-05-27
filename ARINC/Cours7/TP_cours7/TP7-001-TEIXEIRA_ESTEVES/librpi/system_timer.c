
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
#include <libc/stdint.h>          // For uint32_t
#include <librpi/mmap.h>          // For memory map
#include <librpi/interrupts.h>    // For the ICU registers
#include <librpi/system_timer.h>


union SystemTimerCSRegister{
  struct {
    uint32_t M0:1 ;
    uint32_t M1:1 ;
    uint32_t M2:1 ;
    uint32_t M3:1 ;
    uint32_t Reserved:28 ;
  } decoder ;
  uint32_t bv ;
};

struct SystemTimerRegisters{
  // Control/Status
  volatile union SystemTimerCSRegister CS ;
  // System timer counter 64 bits
  volatile uint32_t CLO ;
  volatile uint32_t CHI ;
  // System timer compare 0-3
  volatile uint32_t C0 ;
  volatile uint32_t C1 ;
  volatile uint32_t C2 ;
  volatile uint32_t C3 ;
} ;

static struct SystemTimerRegisters* rpiSystemTimerRegisters = 
  (struct SystemTimerRegisters*)SYSTIMER_BASE;

// I need this quantity for refreshing the timer.
static uint32_t system_timer_period = 0 ;
static uint32_t timeout_value = 0 ;
// Start the timer.
void SystemTimerStart(uint32_t period_us) {
  // According to the BCM manual, page 110, section 7.2, 
  // I have to enable several things.
  // This is the mailbox IRQ, I think, and I enable it,
  // per dwelch's example:
  // https://github.com/dwelch67/raspberrypi/blob/master/blinker07/vectors.s
  rpiICURegisters->Enable_IRQs_1 = 0x02 ;
  // Also, I need to enable the basic IRQ entry.
  union BaseInterruptEnableRegister bier ;
  bier.bv = 0 ;
  bier.decoder.Mailbox = 1 ;
  rpiICURegisters->Enable_Basic_IRQs = bier ;
  // Finally, I set up the timer.
  system_timer_period = period_us ;
  timeout_value = rpiSystemTimerRegisters->CLO ;
  timeout_value += system_timer_period ;
  rpiSystemTimerRegisters->C1 = timeout_value ;
}

// Stop the timer.
void SystemTimerStop() {
  system_timer_period = 0 ;
  timeout_value = 0 ;
  rpiSystemTimerRegisters->C1 = timeout_value ;
}

// Refresh the interrupt register and clear the interrupt.
// Must be called after each timer timeout.
void SystemTimerRefresh() {
  union SystemTimerCSRegister cs ;
  cs.bv = 0 ;
  cs.decoder.M1 = 1 ;
  // Clear the match by over-writing its value.
  rpiSystemTimerRegisters->CS = cs ;
  // Now, write the new timeout value
  timeout_value += system_timer_period ;
  rpiSystemTimerRegisters->C1 = timeout_value ;
}

