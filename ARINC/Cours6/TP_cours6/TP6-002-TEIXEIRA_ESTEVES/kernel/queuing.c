
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
#include <kernel/queuing.h>
#include <kernel/scheduler.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <librpi/debug.h> 

int32_t queuing_send(uint32_t partition_id,
		     uint32_t port_id,
		     struct QueuingMessage* message) {
  int i ;
  /*
  debug_printf("queuing_send: part_id:%d port_id:%d\n"
	       "number_of_channels: %d\n",
	       partition_id,
	       port_id,
	       l1_scheduler_state.number_of_channels) ;
  for(i=0;i<l1_scheduler_state.number_of_channels;i++) {
    debug_printf("   %d : %d:%d -> %d:%d  size:%d\n",
		 i,
		 l1_scheduler_state.channels[i].source_partition_id,
		 l1_scheduler_state.channels[i].source_port_id,
		 l1_scheduler_state.channels[i].dest_partition_id,
		 l1_scheduler_state.channels[i].dest_port_id,
		 l1_scheduler_state.channels[i].max_msg_size) ;
  }
  */
  // Find the channel
  for(i=0;
      (i<l1_scheduler_state.number_of_channels)&&
	((l1_scheduler_state.channels[i].source_partition_id != partition_id)||
	 (l1_scheduler_state.channels[i].source_port_id != port_id)) ; i++) ;
  if(i==l1_scheduler_state.number_of_channels) {
    debug_printf("queuing_send:partition_id:%d port_id:%d\n",
		 partition_id,
		 port_id) ;
    debug_printf("number_of_channels: %d. Channel table:\n",
		 l1_scheduler_state.number_of_channels) ;
    for(i=0;i<l1_scheduler_state.number_of_channels;i++) {
      debug_printf("   %d : %d:%d -> %d:%d  size:%d\n",
		   i,
		   l1_scheduler_state.channels[i].source_partition_id,
		   l1_scheduler_state.channels[i].source_port_id,
		   l1_scheduler_state.channels[i].dest_partition_id,
		   l1_scheduler_state.channels[i].dest_port_id,
		   l1_scheduler_state.channels[i].max_msg_size) ;
    }
    fatal_error("queuing_send:channel not found.\n") ;
  }
  // The channel id is now in i. Check if I there is still place in the
  // queue.
  if(l1_scheduler_state.channels[i].first ==
     (l1_scheduler_state.channels[i].last+1)%CHANNEL_BUFFER_SIZE) {
    // Full queue.
    return NO_ACTION ;
  }
  // Copy data in the cell pointed by last.
  l1_scheduler_state.channels[i].data_buffer[l1_scheduler_state.channels[i].last].size =
    message->size ;
  memcpy(l1_scheduler_state.channels[i].
	 data_buffer[l1_scheduler_state.channels[i].last].data,
	 message->data,
	 message->size) ;
  // Increment the last pointer.
  l1_scheduler_state.channels[i].last =
    (l1_scheduler_state.channels[i].last+1)%CHANNEL_BUFFER_SIZE ;
  return NO_ERROR ;
}

int32_t queuing_recv(uint32_t partition_id,
		     uint32_t port_id,
		     struct QueuingMessage* message) {
  //debug_printf("queuing_recv: part_id:%d port_id:%d\n",partition_id,port_id) ;
  int i ;
  // Find the channel
  for(i=0;
      (i<l1_scheduler_state.number_of_channels)&&
	((l1_scheduler_state.channels[i].dest_partition_id != partition_id)||
	 (l1_scheduler_state.channels[i].dest_port_id != port_id)) ; i++) ;
  if(i==l1_scheduler_state.number_of_channels) {
    fatal_error("queuing_recv:channel not found.\n") ;
  }
  // The channel id is now in i. Check if I the queue is not empty.
  if(l1_scheduler_state.channels[i].first ==
     l1_scheduler_state.channels[i].last) {
    // Empty queue.
    return NO_ACTION ;
  }
  message->size = 
    l1_scheduler_state.channels[i].data_buffer[l1_scheduler_state.channels[i].first].size ;
  memcpy(message->data,
	 l1_scheduler_state.channels[i].
	 data_buffer[l1_scheduler_state.channels[i].first].data,
	 message->size) ;
  // Increment the first pointer.
  l1_scheduler_state.channels[i].first =
    (l1_scheduler_state.channels[i].first+1)%CHANNEL_BUFFER_SIZE ;
  return NO_ERROR ;
}
