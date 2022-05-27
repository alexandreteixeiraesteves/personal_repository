
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
#include <librpi/mmap.h>     // For the position of the registers
#include <librpi/mmio.h>     // For MMIO_READ
#include <librpi/debug.h>    // For the debug routines
#include <libc/assert.h>     // For assert
#include <librpi/mmu.h>
#include <librpi/mbox.h>

//#define mbox_debug_printf(...) debug_printf(__VA_ARGS__)
#define mbox_debug_printf(...)


//===============================================================
// General-purpose mailbox definitions
//===============================================================
/* Data structure. Informations from:
   https://github.com/raspberrypi/firmware/wiki/Mailboxes
   https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
   https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
   www.raspberrypi.org/forums/viewtopic.php?f=34&t=6332
   www.raspberrypi.org/forums/viewtopic.php?t=37395&p=425751

   There are 2 mailboxes. Mailbox 0 is the main one we use.
   It has several channels, defined below.
   The programming is done through a series of registers
   defined by the following data structure (the sets of
   registers are different in the two mboxes).
 */
struct RPiMailBoxRegisters {
  uint32_t MBOX0_READ ;     // 0x00 +4
  uint32_t RESERVED1[3] ;   // 0x04 +4*3
  uint32_t MBOX0_PEEK ;     // 0x10 +4
  uint32_t MBOX0_SENDER ;   // 0x14 +4
  uint32_t MBOX0_STATUS ;   // 0x18 +4
  uint32_t MBOX0_CONFIG ;   // 0x1c +4
  union {
    uint32_t MBOX0_WRITE ;  // 0x20 +4
    uint32_t MBOX1_PEEK ;   // 0x20 +4 (same)
  } c ;  
} ;
// This is the data structure to access the registers.
struct RPiMailBoxRegisters* mbox = 
  (struct RPiMailBoxRegisters*)MBOX_BASE;

// Values to test for checking the STATUS register 
// of MBOX0. To make a write request, the status must 
// be EMPTY (not FULL). To make a read request, the 
// status must be FULL (not EMPTY).
#define MBOX0_STATUS_FULL		0x80000000
#define	MBOX0_STATUS_EMPTY		0x40000000

//===============================================================
// Access protocol for MBOX0
//===============================================================
// We are only using mailbox 0, which has 10 channels.
enum MBOX0Channel {
  MBOX0_PM = 0 ,      // Power Management
  MBOX0_FB = 1,       // Frame Buffer
  MBOX0_VUART = 2,    // Virtual UART
  MBOX0_VCHIQ = 3,    // VCHIQ
  MBOX0_LEDs = 4,     // LEDs
  MBOX0_BUTTONS = 5,  // Buttons
  MBOX0_TS = 6,       // Touch screen
                      // Channel id 7 is unspecified
  MBOX0_PROP = 8,     // Property tags (ARM -> VideoCore)
  MBOX0_PROP1 = 9     // Property tags (VideoCore -> ARM)
} ;
// Write a data on a specific channel of mailbox 0.
// The least significant 4 bits of the data are not 
// transmitted, because they are overwritten with the 
// channel value. 
// It's often the case that the data is a pointer
// to some more data, and in this case the pointer 
// must be  aligned on 16 bytes.
void     mbox0_write(enum MBOX0Channel channel, uint32_t data){
  // Wait until the mbox is empty, thus allowing
  // data writing. This follows a spinlock 
  // protocol.
  while(MMIO_READ(mbox->MBOX0_STATUS) & MBOX0_STATUS_FULL);
  MMIO_WRITE(mbox->c.MBOX0_WRITE, 
	     (data & 0xfffffff0) | (uint32_t)(((uint8_t)channel) & 0xf));
}
// Reading a value on a specific channel.
uint32_t mbox0_read(enum MBOX0Channel channel){
  while(1) {
    // Wait until some data is available. This follows
    // a spinlock protocol.
    while(MMIO_READ(mbox->MBOX0_STATUS) & MBOX0_STATUS_EMPTY);
    // Read the data.
    uint32_t data = MMIO_READ(mbox->MBOX0_READ);
    uint8_t read_channel = (uint8_t)(((uint8_t)data) & 0xf);
    // If it's on the good channel, I keep it. If not,
    // I throw it away and start over again.
    if(read_channel == channel) {
      return (data & 0xfffffff0);
    }
  }
}

//===============================================================
// Access protocol for channel MBOX0_PROP of MBOX0
//===============================================================
// I will be mostly using channel MBOX0_PROP to initiate data 
// transfers from the ARM. This works by exchanging data through
// a buffer with specific position. The buffer is 16-byte aligned,
// cf.https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
//
// The mailbox protocol works by writing uin32_t values of which
// the 28 most significant bits are the address of the buffer (zero 
// padded on the LSB, so aligned on 16 bytes), and the 4 least 
// significant bits are the channel (in our case, 8).
//
// The mailbox buffer has the following structure:
struct MBox0PropBufferHeader {
  uint32_t buffer_size ; // including header, end tag, padding, in bytes
  uint32_t code ;        // request or response code
                         // Valid values:
                         // For requests 0x00000000 (all other are reserved)
                         // For responses:
                         //   - 0x80000000 = request successfull
                         //   - 0x80000001 = error parsing 
                         // all other are reserved.
  // This header is followed by a list of concatenated tags ending with a 
  // (uint32_t)0x0 end tag.
  // Each tag follows the structure defined below and is word aligned.
  // Everything following the 0x0 end tag is ignored.
} ;
// Defined in the struct above.
#define MBOX0_PROP_REQUEST     0x00000000
#define MBOX0_PROP_RSP_SUCCESS 0x80000000
#define MBOX0_PROP_RSP_ERROR   0x80000001


//===============================================================
// Decoding of tags on channel MBOX0_PROP
//===============================================================
// The structure of a tag is the following:
struct MBox0PropTagHeader {
  uint32_t identifier ; // tag identifier
  uint32_t size ;       // buffer size in bytes
  uint32_t composed ;   // 1 MSB: 0=request/1=response
                        //31 LSB: value length in bytes
  // This header is followed by a set of uint32_t words
  // containing the values and potentially some padding.
} ;

// Most often, requests and responses feature a 
// single tag (plus the NULL tag), and then I can use the 
// following struct.
// The max size of the tag data is set to correspond to 
// the choice we made in rpi/mmap.h.
#define MAILBOX_BUFFER_SIZE  (128*sizeof(uint32_t))
#define MAILBOX_BUFFER_ALIGN 16
struct  MBox0PropSingleTagBuffer {
  volatile struct MBox0PropBufferHeader header ;
  volatile struct MBox0PropTagHeader tag_header ;
  volatile uint32_t tag_data[(MAILBOX_BUFFER_SIZE-
			      sizeof(struct MBox0PropBufferHeader)-
			      sizeof(struct MBox0PropTagHeader))
			     /sizeof(uint32_t)] ;
} ;
static struct MBox0PropSingleTagBuffer mbox0_prop_buf
__attribute__((aligned(MAILBOX_BUFFER_ALIGN)));

// Description of the properties that can
// be exchanged on channel MBOX0_PRO
// It helps when printing.
struct PropertyTag {
  const uint32_t identifier ;
  // The size must be large enough to accomodate both
  // request and response length. Normally the max of the
  // two, rounded to be uint32_t-aligned. 
  // All 3 values are provided in bytes.
  const uint32_t size ;
  const uint32_t req_length ;
  const uint32_t rsp_length ;
  // Property name, for printing.
  const char* name ;
};
const struct PropertyTag property_tags[] = {
  // Unknown tag I use for decoding purposes
  {0xFFFFFFFF, 0x00, 0x0, 0x00, "Unknown tag"},                     // 0
  // Hardware
  {0x00000001, 0x04, 0x0, 0x04, "Get firmware revision"},           // 1
  {0x00010001, 0x04, 0x0, 0x04, "Get board model"},                 // 2
  {0x00010002, 0x04, 0x0, 0x04, "Get board revision"},              // 3
  {0x00010003, 0x08, 0x0, 0x06, "Get board MAC address"},           // 4
  {0x00010004, 0x08, 0x0, 0x08, "Get board serial"},                // 5
  {0x00010005, 0x08, 0x0, 0x08, "Get ARM memory"},                  // 6
  {0x00010006, 0x08, 0x0, 0x08, "Get VC memory"},                   // 7
  {0x00010007, 0x40, 0x0, 0x40, "Get clocks (var length)"},         // 8
  // Configuration
  {0x00050001, 0x40, 0x0, 0x40, "Get command line (var length)"},   // 9
  // Shared resource management
  {0x00060001, 0x04, 0x0, 0x04, "Get DMA channels"},                //10
  {0x00020001, 0x08, 0x4, 0x08, "Get power state"},                 //11
  {0x00020002, 0x08, 0x4, 0x08, "Get timing"},                      //12
  {0x00028001, 0x08, 0x8, 0x08, "Set power state"},                 //13
  // Clocks
  {0x00030001, 0x08, 0x4, 0x08, "Get clock state"},                 //14
  {0x00038001, 0x08, 0x8, 0x08, "Set clock state"},                 //15
  {0x00030002, 0x08, 0x4, 0x08, "Get clock rate"},                  //16
  {0x00038002, 0x08, 0x8, 0x08, "Set clock rate"},                  //17
  {0x00030004, 0x08, 0x4, 0x08, "Get max clock rate"},              //18
  {0x00030007, 0x08, 0x4, 0x08, "Get min clock rate"},              //19
  {0x00030009, 0x08, 0x4, 0x08, "Get turbo"},                       //20
  {0x00038009, 0x08, 0x8, 0x08, "Set turbo"},                       //21
  // Other: Voltage, Memory allocation.
  // Execution, Framebuffer
  // TODO, depending on the needs
  //
  // Finally, the NULL tag
  // It MUST remain the last in this list, as it acts as 
  // a terminator.
  {0x00000000, 0x00, 0x0, 0x00, "NULL"}
} ;
// Rapid access to this table (since identifiers are 
// not contiguous).
enum MBox0PropTags {
  MBOX0_PROP_TAG_UNKNOWN               = 0,
  MBOX0_PROP_TAG_GET_FIRMWARE_REV      = 1,
  MBOX0_PROP_TAG_GET_BOARD_MODEL       = 2,
  MBOX0_PROP_TAG_GET_BOARD_REV         = 3,
  MBOX0_PROP_TAG_GET_MAC_ADDRESS       = 4,
  MBOX0_PROP_TAG_GET_BOARD_SERIAL      = 5,
  MBOX0_PROP_TAG_GET_ARM_RAM           = 6,
  MBOX0_PROP_TAG_GET_VC_RAM            = 7,
  MBOX0_PROP_TAG_GET_CLOCKS            = 8,
  MBOX0_PROP_TAG_GET_CMDLINE           = 9,
  MBOX0_PROP_TAG_GET_DMA_CHANNELS      = 10,
  MBOX0_PROP_TAG_GET_PWR_STATE         = 11,
  MBOX0_PROP_TAG_GET_TIMING            = 12,
  MBOX0_PROP_TAG_SET_PWR_STATE         = 13,
  MBOX0_PROP_TAG_GET_CLK_STATE         = 14,
  MBOX0_PROP_TAG_SET_CLK_STATE         = 15,
  MBOX0_PROP_TAG_GET_CLK_RATE          = 16,
  MBOX0_PROP_TAG_SET_CLK_RATE          = 17,
  MBOX0_PROP_TAG_GET_MAX_CLK_RATE      = 18,
  MBOX0_PROP_TAG_GET_MIN_CLK_RATE      = 19,
  MBOX0_PROP_TAG_GET_TURBO             = 20,
  MBOX0_PROP_TAG_SET_TURBO             = 21,
  MBOX0_PROP_TAG_NULL                  = 22 // This one must always remain 
                                            // the last.
} ;

// Starting from a tag identifier from the RPi
// specification, obtain the identifier that 
// allows access to our data structures. 
uint32_t MBox0PropIdentifyTag(uint32_t tag_identifier) {
  uint32_t i ;
  // If it's a NULL tag, return.
  if(tag_identifier == 0) return MBOX0_PROP_TAG_NULL ;
  for(i=1;property_tags[i].identifier != 0x00000000;i++) {
    // If it is not NULL, and if we have it defined, then
    // return it.
    if(tag_identifier == property_tags[i].identifier) {
      return i ;
    }
  }
  // All other tags are unknown.
  return MBOX0_PROP_TAG_UNKNOWN ;
}

// Pretty print an MBox buffer 
void 
DebugPrintMBox0PropBuffer(const volatile struct MBox0PropBufferHeader * buf) {
  mbox_debug_printf("==========================================\r\n"
	       "MBox0PropBuffer: SIZE=%d  TYPE=",
	       buf->buffer_size) ;
  switch(buf->code) {
  case MBOX0_PROP_REQUEST: mbox_debug_printf("REQ") ; break ;
  case MBOX0_PROP_RSP_SUCCESS: mbox_debug_printf("RSP_SUCCESS") ; break ;
  case MBOX0_PROP_RSP_ERROR: mbox_debug_printf("RSP_ERROR") ; break ;
  default:
    mbox_debug_printf("ERRONEOUS TYPE... Aborting") ;
    return ;
  }
  mbox_debug_printf("\r\n") ;
  // Make a parallel accounting of the structure size
  // in order to make a sanity check.
  uint32_t size_counter = 0 ;
  size_counter += sizeof(struct MBox0PropBufferHeader) ;
  // Now, skip the mbox header.
  const struct MBox0PropTagHeader* tag_buf = 
    (const struct MBox0PropTagHeader*)(buf + 1) ;
  // A counter for the tags, for printing purposes
  int tag_cnt = 0 ;
  do {
    mbox_debug_printf("\tTAG%d:",tag_cnt) ;
    uint32_t decoded_identifier = MBox0PropIdentifyTag(tag_buf->identifier);
    mbox_debug_printf("%#x(%s) ",
		 tag_buf->identifier,
		 property_tags[decoded_identifier].name) ;
    if(decoded_identifier == MBOX0_PROP_TAG_NULL) {
      // Update the size counter. This tag only takes one word.
      size_counter += sizeof(uint32_t) ;
      // This is the NULL tag. Nothing more to print 
      // except the end of the line.
      mbox_debug_printf("\r\n------------------------------------------\n\r") ;
      return ;
    } else {
      // Not the null tag. Print its contents and advance.
      mbox_debug_printf("BUFSIZE:%x IsRSP:%d DATALEN:%x DATA:",
		   tag_buf->size,
		   ((tag_buf->composed&0x80000000)?1:0),
		   tag_buf->composed&0x7fffffff) ;
      // Account for the tag header
      size_counter += sizeof(struct MBox0PropTagHeader) ;
      // Point to the start of the data
      const unsigned char* data = (unsigned char*)(tag_buf+1) ;
      UNUSED(data) ; // for the case where I don't print it.
      const uint32_t* data_words = (uint32_t*)(tag_buf+1) ;
      { 
	// Print the data and the padding.
	uint32_t j ;
	for(j=0;j<tag_buf->size;j++) {
	  if(!(j&0x3)){
	    mbox_debug_printf("|") ;
	  }
	  if(j<(tag_buf->composed&0x7fffffff)) {
	    // These are data bytes.
	    mbox_debug_printf("%2x",data[j]) ;
	  } else {
	    // These are padding bytes.
	    mbox_debug_printf("XX") ;
	  }
	}
	mbox_debug_printf("|") ;
      }
      //Finish the tag line.
      mbox_debug_printf("\n\r") ;
      // Account for the data size. I have to make a conversion
      // to uint32_t. The documentation states that the buffer
      // is word-aligned, so I just have to shift,
      assert(!(tag_buf->size&0x3)) ;
      uint32_t buffer_size_in_words = (tag_buf->size>>2) ;
      // Account for the data buffer.
      size_counter += tag_buf->size ;
      // And now, move to the next tag.
      tag_buf = 
	(const struct MBox0PropTagHeader*)(data_words + buffer_size_in_words) ;
      tag_cnt ++ ;
    } // endif
  } while (size_counter<buf->buffer_size) ;
  // The condition of the while loop is a sanity check meant to ensure
  // that the whole process ends at some point, but if
  // I got here there's an error.
  mbox_debug_printf("MBox buffer is corrupt. Returning...\n\r");;
}

void MBox0PropBuildBufferHeaders(enum MBox0PropTags tag) {
  mbox0_prop_buf.header.buffer_size = 
    // Buffer header
    sizeof(struct MBox0PropBufferHeader) +
    // Request tag header
    sizeof(struct MBox0PropTagHeader) +
    // Request tag data
    property_tags[tag].size +
    // The size of a NULL tag
    sizeof(uint32_t) ; // The size of a NULL tag
  mbox0_prop_buf.header.code = MBOX0_PROP_REQUEST ;
  mbox0_prop_buf.tag_header.identifier = 
    property_tags[tag].identifier ;
  mbox0_prop_buf.tag_header.size = 
    property_tags[tag].size ;
  mbox0_prop_buf.tag_header.composed = 
    property_tags[tag].req_length ;
} 

void MBox0PropMakeMBoxExchange() {
  mbox_debug_printf("MBox0PropMakeMBoxExchange entered\n") ;
  // Request build finished. Print it.
  DebugPrintMBox0PropBuffer(&(mbox0_prop_buf.header)) ;
  // Send the request and wait until a response arrives.
  mbox0_write(MBOX0_PROP,(uint32_t)&mbox0_prop_buf) ;
  dcache_flush_memory_range((char*)&mbox0_prop_buf,MAILBOX_BUFFER_SIZE) ;
  //  clean_invalidate_data_cache() ;
  mbox0_read(MBOX0_PROP) ;
  //  cache_flush_memory_range((char*)mbox0_prop_buf,MAILBOX_BUFFER_SIZE) ;
  //clean_invalidate_data_cache() ;
  // Invalidate the data cache to make sure the data is
  // updated.
  // It does not work, so I comment it.
  //  clean_invalidate_data_cache() ;
  // Print the response.      
  DebugPrintMBox0PropBuffer(&(mbox0_prop_buf.header)) ;
  mbox_debug_printf("MBox0PropMakeMBoxExchange completed\n") ;
}

// Return the power state of the device.
// Return values:
// -1 = error in reading the state
//  0 = device off
//  1 = device on
int MBoxGetPowerState(enum MBox0PropDeviceList device_id) {
  // Construct the request in the buffer.
  MBox0PropBuildBufferHeaders(MBOX0_PROP_TAG_GET_PWR_STATE) ;
  // Target device
  mbox0_prop_buf.tag_data[0] = (uint32_t)device_id ;
  // The second word is padding to make place for 
  // the return value. I don't initialize it.
  // The third word is the NULL tag, that comes after the
  // first tag.
  mbox0_prop_buf.tag_data[2] = 
    property_tags[MBOX0_PROP_TAG_NULL].identifier ;
  // Make MBox exchange
  MBox0PropMakeMBoxExchange() ;
  // Several variants are possible
  if(mbox0_prop_buf.tag_data[1]&0x2) {
    // Device does not exist error
    return -1 ;
  } else {
    // Return the activation bit.
    return mbox0_prop_buf.tag_data[1]&0x1 ;
  }
}

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
		      int activation_flag) {
  // Construct the request in the buffer.
  MBox0PropBuildBufferHeaders(MBOX0_PROP_TAG_SET_PWR_STATE) ;
  // Target device
  mbox0_prop_buf.tag_data[0] = (uint32_t)device_id ;
  // The second word is the desired state and the wait flag.
  mbox0_prop_buf.tag_data[1] = 
    (wait_flag?0x2:0x0)|(activation_flag?0x1:0x0) ;
  // The third word is the NULL tag, that comes after the
  // first tag.
  mbox0_prop_buf.tag_data[2] = 
    property_tags[MBOX0_PROP_TAG_NULL].identifier ;
  // Make MBox exchange
  MBox0PropMakeMBoxExchange() ;
  // Several variants are possible
  if(mbox0_prop_buf.tag_data[1]&0x2) {
    // Device does not exist error
    return -1 ;
  } else {
    // Return the new activation bit.
    return mbox0_prop_buf.tag_data[1]&0x1 ;
  }
}

// Returns the rate in Hz of a given clock, or 
// 0 if such a clock does not exist.
uint32_t MBoxGetClockRate(enum MBox0PropClockList clk_id) {
  // Construct the request in the buffer.
  MBox0PropBuildBufferHeaders(MBOX0_PROP_TAG_GET_CLK_RATE) ;
  // Target device
  mbox0_prop_buf.tag_data[0] = (uint32_t)clk_id ;
  // The second word is padding to make place for 
  // the return value. I don't initialize it.
  // The third word is the NULL tag, that comes after the
  // first tag.
  mbox0_prop_buf.tag_data[2] = 
    property_tags[MBOX0_PROP_TAG_NULL].identifier ;
  // Make MBox exchange
  MBox0PropMakeMBoxExchange() ;
  // Return the value, which will either be a correct
  // frequency, or 0 i case the clock does not exist.
  return mbox0_prop_buf.tag_data[1] ;
}
