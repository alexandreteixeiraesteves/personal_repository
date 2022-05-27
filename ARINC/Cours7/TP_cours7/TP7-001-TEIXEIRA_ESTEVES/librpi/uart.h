
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
#ifndef UART_H
#define UART_H
 
#include <libc/stdint.h>

//----------------------------------------------------------------
// Initialize UART0.
void uart_init(void);

//----------------------------------------------------------------
// SENDING

// Transmit a byte via UART0.
void uart_putc(uint8_t byte);

// Print a string to the UART one character at a time.
// No reformatting at all.
void uart_puts(const char *str);

//----------------------------------------------------------------
// RECEIVING

// Receives a byte via UART0. Block until one is received.
uint8_t uart_getc(void);

// Non-blocking read of one byte over UART0.
// If a byte has been received, return it. If not, do not
// block.
// Return value:  
// A 2-byte word whose least significant byte is the potentially received
// character, and the most significant byte is non-zero if 
// nothing has been received, and zero if something has been received.
uint16_t uart_getc_nonblocking(void);

#endif
