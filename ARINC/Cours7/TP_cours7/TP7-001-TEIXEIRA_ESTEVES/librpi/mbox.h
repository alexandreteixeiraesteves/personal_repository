
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
#ifndef MBOX_H
#define MBOX_H

#include <libc/stdint.h>

// List of unique device IDs needed in formulating requests.
enum MBox0PropDeviceList{
  MBOX0_PROP_DEVICE_SDCARD = 0x00000000,
  MBOX0_PROP_DEVICE_UART0  = 0x00000001,
  MBOX0_PROP_DEVICE_UART1  = 0x00000002,
  MBOX0_PROP_DEVICE_USBHCD = 0x00000003,
  MBOX0_PROP_DEVICE_I2C0   = 0x00000004,
  MBOX0_PROP_DEVICE_I2C1   = 0x00000005,
  MBOX0_PROP_DEVICE_I2C2   = 0x00000006,
  MBOX0_PROP_DEVICE_SPI    = 0x00000007,
  MBOX0_PROP_DEVICE_CCP2TX = 0x00000008
} ;

// List of unique clock identifiers needed in formulating
// requests.
enum MBox0PropClockList{
  MBOX0_PROP_CLK_RESERVED = 0x000000000,
  MBOX0_PROP_CLK_EMMC     = 0x000000001,
  MBOX0_PROP_CLK_UART     = 0x000000002,
  MBOX0_PROP_CLK_ARM      = 0x000000003,
  MBOX0_PROP_CLK_CORE     = 0x000000004,
  MBOX0_PROP_CLK_V3D      = 0x000000005,
  MBOX0_PROP_CLK_H264     = 0x000000006,
  MBOX0_PROP_CLK_ISP      = 0x000000007,
  MBOX0_PROP_CLK_SDRAM    = 0x000000008,
  MBOX0_PROP_CLK_PIXEL    = 0x000000009,
  MBOX0_PROP_CLK_PWM      = 0x00000000a
};

// Return the power state of the device.
// Return values:
// -1 = error in reading the state
//  0 = device off
//  1 = device on
int MBoxGetPowerState(enum MBox0PropDeviceList device_id) ;


// Return the power state of the device.
// Input values:
//   activation_flag - the desired state of the device
//                   - false (0) = inactive, true = active
//   wait_flag - if this flag is set, the mailbox return
//               comes after stabilization of the power.
// Return value:
//  1 = success (the state after call is the correct one)
//  0 = failure (the state after call is not correct)
int MBoxSetPowerState(enum MBox0PropDeviceList device_id, 
		      int wait_flag,
		      int activation_flag);

// Returns the rate in Hz of a given clock, or 
// 0 if such a clock does not exist. The input is 
// a clock id from the above enum.
uint32_t MBoxGetClockRate(enum MBox0PropClockList clk_id) ;


#endif

