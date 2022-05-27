
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
#include <libc/string.h>       // For strncpy
#include <arinc653/error.h>    // For NO_ERROR
#include <kernel/errors.h>     // For error codes.
#include <libc/util.h>         // For increment_modulo
#include <librpi/registers.h>  // For enable/disable_interrupts
#include <librpi/uart.h>       // For uart_puts
#include <libc/stdio.h>        // For snprintf

#define MAX_PRINTF_MSG_SIZE 128
#define PRINTF_BUFFER_SIZE  128
struct {
  char msg_buffer[MAX_PRINTF_MSG_SIZE][PRINTF_BUFFER_SIZE] ;
  uint32_t first ;
  uint32_t last ;
  uint32_t niveau_vanne ;
} console_state ;
// Convention on the console state:
// - first == last                        => empty queue
// - first == (last+1)%PRINTF_BUFFER_SIZE => full queue

// Enqueue messages for console logging.
// Called from the SVC handler. Must run under the DACR memory access
// permissions of the calling partition, in order to access the msg
// string.
uint32_t console_log_message(char*msg) {
  uint32_t new_last =
    increment_modulo(console_state.last,PRINTF_BUFFER_SIZE) ;
  if(new_last == console_state.first) {
    // Full queue, cannot enqueue.
    return CONSOLE_OVERFLOW ;
  }
  // Copy the message -- mandatory, since the console driver does not
  // have access to partition memory.
  strncpy(console_state.msg_buffer[console_state.last],
	  msg,
	  MAX_PRINTF_MSG_SIZE-1) ;
  // Make sure it's NULL-terminated.
  console_state.msg_buffer[console_state.last][MAX_PRINTF_MSG_SIZE-1]='\0';
  console_state.last = new_last ;
  return NO_ERROR ;
}

// Initialize the console (nothing to do, actually, the structures
// are well-initialized by bss zeroing, but I do it again for the
// sake of example).
void console_init() {
  console_state.first = 0 ;
  console_state.last = 0 ;
  console_state.niveau_vanne = 0 ;
}

// Routine called from the system partition.
// Logs one message on the console. It should be
// called in a loop with the other drivers.
void console_driver() {
  // First, check if some input is available on the
  // serial port.
  disable_interrupts() ;
  uint16_t recv_byte = uart_getc_nonblocking() ;
  enable_interrupts() ;
  if(recv_byte) {
    switch(recv_byte&0xff) {
    case 'o': 
      if(console_state.niveau_vanne<10) {
	console_state.niveau_vanne += 1 ;
      } 
      break ;
    case 'f':
      if(console_state.niveau_vanne>0) {
	console_state.niveau_vanne -= 1 ;
      } 
      break ;
    default:
      {
	char msg[256] ;
	snprintf(msg,255,"XXXX from host: 0x%x.\n",recv_byte) ; 
	console_log_message(msg) ;
      }
      break ;
    }
  }
  // Second, if some 
  if(console_state.last != console_state.first) {
    disable_interrupts() ;
    uart_puts("CONSOLE:") ;
    uart_puts(console_state.msg_buffer[console_state.first]);
    enable_interrupts() ;
    console_state.first =
      increment_modulo(console_state.first,PRINTF_BUFFER_SIZE) ;
  }
}

uint32_t console_read_level() {
  return console_state.niveau_vanne ;
}
