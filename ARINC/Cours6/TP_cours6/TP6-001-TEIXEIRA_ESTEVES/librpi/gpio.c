
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
#include <librpi/mmap.h>
#include <librpi/mmio.h>
#include <librpi/gpio.h>

/* GPIO Register set */
/* Assign the address of the GPIO peripheral (Using ARM Physical
   Address) */
struct GPIORegisters* gpio = (struct GPIORegisters*)GPIO_BASE ;


/* Setting and clearing a GPIO under and over 32.

   The GPSET and GPCLR words are used to actually set the value
   of the GPIOs to either 0 (CLR) or 1 (SET). The 2 SET 
   words should be seen as a 64-bit bit vector where the 
   first 47 bits are used to set the various GPIO. Similarly
   for the 2 CLR words.
   I wonder if I can SET or CLR multiple GPIO simultaneously.
*/
void gpio_set_value(unsigned int gpio_id,unsigned int value) {
  unsigned int selector = gpio_id/32 ; //32 = sizeof(int)
  unsigned int mask = 1 << (gpio_id%32) ;
  if (value) MMIO_WRITE(gpio->GPSET[selector], mask) ;
  else MMIO_WRITE(gpio->GPCLR[selector], mask) ;
}

/* Initializing a GPIO as input or output. The argument g is the
   number f the GPIO. It can be greater than 32.

   A GPIO is set to be an input by setting the 3 corresponding bits on
   the corresponding GPFSEL word to 0b000 (and not changing the
   other). Attention, 0b000 corresponds to ~0b111, which explains the
   constant 7 in the code. It is set to be an output by setting the 3
   bits to 0b001. I wonder what the other combinations (001, 010,
   011,...) are doing.
*/
void gpio_init_input(unsigned int gpio_id) {
  unsigned int selector = gpio_id/10 ;
  unsigned int mask = ~(7 << ((gpio_id%10)*3)) ;
  // The following line does gpio->GPFSEL[selector] &= mask;
  // but under MMIO constraints. Directly using the simple
  // expression is not correct in the general case because
  // gcc apparently can optimize out some accesses.
  MMIO_WRITE(gpio->GPFSEL[selector],MMIO_READ(gpio->GPFSEL[selector])&mask) ;
}
void gpio_init_output(unsigned int gpio_id) {
  unsigned int selector = gpio_id/10 ;
  unsigned int mask = 1 << ((gpio_id%10)*3) ;
  // The following line does gpio->GPFSEL[selector] |= mask;
  // but under MMIO constraints. Directly using the simple
  // expression is not correct in the general case because
  // gcc apparently can optimize out some accesses.
  MMIO_WRITE(gpio->GPFSEL[selector],MMIO_READ(gpio->GPFSEL[selector])|mask) ;
}
