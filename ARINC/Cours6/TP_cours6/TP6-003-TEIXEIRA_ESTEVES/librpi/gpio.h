
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
#ifndef GPIO_H
#define GPIO_H

#include <libc/stdint.h>

// Used to set the direction (and possibly other
// properties) of GPIO. Each unsigned int value 
// GPIO_BASE[GPIO_GPFSELn] for n=0..3 stores the
// properties of 10 GPIO, as a sequence of 3 bits
// per GPIO. This means that 2 bits remain unused.
// GPIOs 40 to 46 are stored on 
// GPIO_BASE[GPIO_GPFSEL4], but don't fill it.
// I don't know what the remaining GPFSEL5 is doing
// (maybe provision for future extension).
//
// ATTENTION: accessing the GPIO registers as I do 
// in gpio.c (without using the routines from 
// mmio.h) is only possible if the memory area 
// of the memory-mapped devices is UNCACHED.
// The RPI has all data accesses uncached by 
// default, but care must be taken once the MMU
// is activated so that all memory-mapped I/O are
// uncached.
struct GPIORegisters {
  volatile uint32_t GPFSEL[6];     // 0x00 + 4*6
  volatile uint32_t Reserved1;     // 0x18 + 4
  volatile uint32_t GPSET[2];      // 0x1c + 4*2
  volatile uint32_t Reserved2;     // 0x24 + 4
  volatile uint32_t GPCLR[2];      // 0x28 + 4*2
  volatile uint32_t Reserved3;     // 0x30 + 4
  //JP: I don't know what the following are doing.
  //    I just put them here for completeness.
  volatile uint32_t GPLEV[2];      // 0x34 + 4*2
  volatile uint32_t Reserved4;     // 0x3c + 4
  volatile uint32_t GPEDS[2];      // 0x40 + 4*2
  volatile uint32_t Reserved5;     // 0x48 + 4 
  volatile uint32_t GPREN[2];      // 0x4c + 4*2
  volatile uint32_t Reserved6;     // 0x54 + 4
  volatile uint32_t GPFEN[2];      // 0x58 + 4*2
  volatile uint32_t Reserved7;     // 0x60 + 4
  volatile uint32_t GPHEN[2];      // 0x64 + 4*2
  volatile uint32_t Reserved8;     // 0x6c + 4
  volatile uint32_t GPLEN[2];      // 0x70 + 4*2
  volatile uint32_t Reserved9;     // 0x78 + 4 
  volatile uint32_t GPAREN[2];     // 0x7c + 4*2
  volatile uint32_t Reserved10;    // 0x84 + 4
  volatile uint32_t GPAFEN[2];     // 0x88 + 4*2
  volatile uint32_t Reserved11;    // 0x90 + 4
  volatile uint32_t GPPUD;         // 0x94 + 4
  volatile uint32_t GPPUDCLK[2];   // 0x98 + 4*2
};

/* GPIO Register set */
/* Assign the address of the GPIO peripheral (Using ARM Physical
   Address) */
extern struct GPIORegisters* gpio ;

// Generic access functions
void gpio_init_input(unsigned int gpio_id) ;
void gpio_init_output(unsigned int gpio_id) ;
void gpio_set_value(unsigned int gpio_id,unsigned int value) ;

// The two particular GPIOs we use
#define GPIO_LED_GREEN   47
#define GPIO_LED_RED     35

#endif
