
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
#include <librpi/timer.h>
#include <librpi/uart.h>
 
// The UART0 registers, cf. 
// http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals
// chapter 13.
//
// ATTENTION: accessing the UART0 registers as I do 
// in gpio.c (without using the routines from 
// mmio.h) is only possible if the memory area 
// of the memory-mapped devices is UNCACHED.
// The RPI has all data accesses uncached by 
// default, but care must be taken once the MMU
// is activated so that all memory-mapped I/O are
// uncached.
struct UART0Registers {
  volatile uint32_t DR ;          //0x00 + 4
  volatile uint32_t RSRECR ;      //0x04 + 4
  volatile uint32_t UNUSED1[4] ;  //0x08 + 4*4
  volatile uint32_t FR ;          //0x18 + 4
  volatile uint32_t UNUSED2 ;     //0x1c + 4
  volatile uint32_t ILPR ;        //0x20 + 4
  volatile uint32_t IBRD ;        //0x24 + 4
  volatile uint32_t FBRD ;        //0x28 + 4
  volatile uint32_t LCRH ;        //0x2c + 4
  volatile uint32_t CR ;          //0x30 + 4
  volatile uint32_t IFLS ;        //0x34 + 4
  volatile uint32_t IMSC ;        //0x38 + 4
  volatile uint32_t RIS ;         //0x3c + 4
  volatile uint32_t MIS ;         //0x40 + 4
  volatile uint32_t ICR ;         //0x44 + 4
  volatile uint32_t DMACR ;       //0x48 + 4
  volatile uint32_t UNUSED3[13] ; //0x4c + 4*13
  volatile uint32_t ITCR ;        //0x80 + 4
  volatile uint32_t ITIP ;        //0x84 + 4
  volatile uint32_t ITOP ;        //0x88 + 4
  volatile uint32_t TDR ;         //0x8c + 4
};

/* UART0 register set */
/* Assign the address of the UART0 peripheral (Using ARM Physical
   Address) */
struct UART0Registers* uart0 = 
  (struct UART0Registers*)UART0_BASE ;

/*
 * delay function
 * int32_t delay: number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * wont optimize away. Note that this is not real time because
 * the clock frequency may vary. However, it's handy to have 
 * a very simple delay routine that does not require timers.
 */
//static void delay_cycles(int32_t count) {
//  asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
//	       : : [count]"r"(count) : "memory", "cc");
//}
 
// Initialize UART0.
void uart_init() {
  // Disable UART0.
  MMIO_WRITE(uart0->CR, 0x00000000) ;
  // Setup the GPIO pin 14 && 15.
  // Disable pull up/down for all GPIO pins & delay for 150 cycles.
  MMIO_WRITE(gpio->GPPUD, 0x00000000) ;
  usleep(1000); // Give time to write the possible current 
  // character in the pipe...
  // Disable pull up/down for pin 14,15 & delay for some time.
  MMIO_WRITE(gpio->GPPUDCLK[0],(1 << 14)|(1 << 15));
  usleep(150); 
  // Write 0 to GPPUDCLK0 to make it take effect.
  MMIO_WRITE(gpio->GPPUDCLK[0],0x00000000);
  // Clear pending interrupts.
  MMIO_WRITE(uart0->ICR, 0x7FF) ;
  // Set integer & fractional part of baud rate.
  // Divider = UART_CLOCK/(16 * Baud)
  // Fraction part register = (Fractional part * 64) + 0.5
  // UART_CLOCK = 3000000; Baud = 115200.  
  // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
  // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
  MMIO_WRITE(uart0->IBRD, 1);
  MMIO_WRITE(uart0->FBRD, 40);
  // Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
  MMIO_WRITE(uart0->LCRH, (1 << 4) | (1 << 5) | (1 << 6)) ;
  // Mask all interrupts.
  MMIO_WRITE(uart0->IMSC,
	     (1 << 1) | (1 << 4) | (1 << 5) |
	     (1 << 6) | (1 << 7) | (1 << 8) |
	     (1 << 9) | (1 << 10));  
  // Enable UART0, receive & transfer part of UART.
  MMIO_WRITE(uart0->CR, (1 << 0) | (1 << 8) | (1 << 9));     
}
 
/*
 * Transmit a byte via UART0.
 * uint8_t Byte: byte to send.
 */
void uart_putc(uint8_t byte) {
  // wait for UART to become ready to transmit
  while (1) {
    if (!(uart0->FR & (1 << 5))) break ;
  }
  uart0->DR = byte ;
}
 
/*
 * print a string to the UART one character at a time
 * const char *str: 0-terminated string
 */
void uart_puts(const char *str) {
  while (*str) {
    if(*str == '\n')uart_putc('\r') ;
    uart_putc(*str++);
  }
}

/*
 * Receive a byte via UART0.
 *
 * Returns:
 * uint8_t: byte received.
 */
uint8_t uart_getc() {
  // wait for UART to have received something
  while(MMIO_READ(uart0->FR) & (1 << 4)) ;
  return MMIO_READ(uart0->DR);
}

uint16_t uart_getc_nonblocking() {
  if(MMIO_READ(uart0->FR) & (1 << 4)) {
    return (uint16_t)0x0000 ;
  } else {
    return
      ((uint16_t)0xff00)|((uint16_t)MMIO_READ(uart0->DR));
  }
}
