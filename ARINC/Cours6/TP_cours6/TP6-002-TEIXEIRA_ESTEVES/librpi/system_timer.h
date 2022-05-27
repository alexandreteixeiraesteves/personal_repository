
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
#include <libc/stdint.h>

// The system timer is supposedly a timer with 
// 1ms period (1MHz frequency). 

// Start the timer by providing its period.
void SystemTimerStart(uint32_t period_ms) ;
// Stop the timer.
void SystemTimerStop(void) ;
// Refresh the interrupt register and clear the interrupt.
// Must be called after each timer timeout.
void SystemTimerRefresh(void) ;
