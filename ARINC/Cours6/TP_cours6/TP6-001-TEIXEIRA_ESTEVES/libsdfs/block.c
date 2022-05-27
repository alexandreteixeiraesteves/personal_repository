
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
/* This file of RPi653 is based on the rpi-boot 
   software by John Cronin. Copyright notice of this
   software is provided below.
 */
   
/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <libc/stdint.h>
#include <librpi/debug.h>
#include <libc/arm-eabi.h>
#include <libsdfs/fs.h>
#include <libc/stdlib.h>
#include <librpi/timer.h>

//#define block_debug_printf(...) debug_printf(__VA_ARGS__)
#define block_debug_printf(...)


#define MAX_TRIES		1

size_t block_read(struct block_device *dev, uint8_t *buf, 
		  size_t buf_size, uint32_t starting_block) {
  // Read the required number of blocks to satisfy the request
  int buf_offset = 0;
  uint32_t block_offset = 0;
  
  if(!dev->read)
    return 0;
  
  // Perform a multi-block read if the device supports it
  if(dev->supports_multiple_block_read && (rpi_uidiv(buf_size, dev->block_size) > 1)) {
    block_debug_printf("block_read: performing multi block read (%d blocks) from "
		       "block %d on %s\n", 
		       rpi_uidiv(buf_size, dev->block_size), starting_block,
		       dev->device_name);
    //if(starting_block==20512){DebugPrintHeapOrganization() ;}
    size_t retval = dev->read(dev, buf, buf_size, starting_block);
    block_debug_printf("block_read: read returned with value %d\n",
		 retval) ;
    //if(starting_block==20512){DebugPrintHeapOrganization() ;}
    //usleep(5000) ;
    //asm volatile("":::"memory") ;
    return retval ;
  }

  do {
    size_t to_read = buf_size;
    if(to_read > dev->block_size)
      to_read = dev->block_size;    
    block_debug_printf("block_read: reading %d bytes from block %d on %s\n", 
		 to_read,
		 starting_block + block_offset, dev->device_name);
    int tries = 0;
    while(1) {
      int ret = dev->read(dev, &buf[buf_offset], to_read,
			  starting_block + block_offset);
      if(ret < 0) {
	tries++;
	if(tries >= MAX_TRIES)
	  return ret;
      }
      else
	break;
    }
    
    buf_offset += (int)to_read;
    block_offset++;
    
    if(buf_size < dev->block_size)
      buf_size = 0;
    else
      buf_size -= dev->block_size;
  } while(buf_size > 0);
  
  return (size_t)buf_offset;
}

size_t block_write(struct block_device *dev, uint8_t *buf, 
		   size_t buf_size, uint32_t starting_block) {
  // Write the required number of blocks to satisfy the request
  int buf_offset = 0;
  uint32_t block_offset = 0;
  
  if(!dev->write)
    return 0;
  
  do
    {
      size_t to_write = buf_size;
      if(to_write > dev->block_size)
	to_write = dev->block_size;
      
      block_debug_printf("block_write: writing %d bytes to block %d on %s\n", 
		   to_write,
		   starting_block + block_offset, dev->device_name);
      
      int tries = 0;
      while(1) {
	int ret = dev->write(dev, &buf[buf_offset], to_write,
			     starting_block + block_offset);
	if(ret < 0)
	  {
	    tries++;
	    if(tries >= MAX_TRIES)
	      return ret;
	  }
	else
	  break;
      }
      
      buf_offset += (int)to_write;
      block_offset++;
      
      if(buf_size < dev->block_size)
	buf_size = 0;
      else
	buf_size -= dev->block_size;
    } while(buf_size > 0);
  
  return (size_t)buf_offset;
}
