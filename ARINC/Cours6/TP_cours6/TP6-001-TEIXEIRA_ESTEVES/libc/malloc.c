
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
#include <librpi/mmap.h>      // For the memory limits
#include <librpi/mmap_c.h>    // For the memory limits
#include <librpi/debug.h>     // For debug_puts, fatal_error
                              // and warning
#include <libc/stdio.h>       // For uint32ascii
#include <libc/arm-eabi.h>    // For the div/mod functions
#include <libc/errno.h>       // For errno
#include <libc/stdlib.h>

// Memory is structured as a continuous list of memory 
// blocks which can be either allocated or non-allocated.
//
// Non-allocated blocks are stored under the form of
// a doubly-linked list (each non-allocated block contains
// two pointers, named prev and next, to the non-allocated 
// block before and after it).
// 
// All pointers involved in allocation are word-aligned.
// To faciltate allocation, I require that block lengths are 
// a multiple of 4 words, because the smallest free block
// has 4 words (length, prev, next, start).  For this reason, 
// all blocks are aligned on 4 uint32_t.
// For each block:
// - the last uint32_t of each block (allocated or not) 
//   is a pointer to its start. 
// - the first uint32_t of each block is a packed bitvector
//   of type struct ALBitvector, of which:
//   * the first bit says whether it's allocated or not
//   * the last 28 bits give the length of the block, in
//     4-word sections (this length includes the size of 
//     the header and the trailing pointer to the start). 
//     Given this alignment, 28 bits are enough to cover
//     a 32-bit memory space.
//   * between the 2 there remain 3 unused bits.
// To facilitate conversions, I define these macros
__attribute__((always_inline))
inline uint32_t chunks2bytes(uint32_t chunks) {
  return chunks<<4 ;
}
__attribute__((always_inline))
inline uint32_t bytes2chunks(uint32_t bytes) {
  return bytes>>4 ;
}
__attribute__((always_inline))
inline uint32_t chunks2words(uint32_t chunks) {
  return chunks<<2 ;
}
__attribute__((always_inline))
inline uint32_t words2chunks(uint32_t words) {
  return words>>2 ;
}

// This header is used to decode the header of all blocks.
struct MemoryBlockHeader {
  // Defined for both allocated and non-allocated memory blocks.
  // The least significant bit defines allocation. The 30 most
  // significant ones define the length. 
  struct ALBitvector{
    uint32_t allocated :  1 ; 
    uint32_t unused    :  3 ; 
    uint32_t length    : 28 ; // In 4-word (16-byte) chunks
  } al ; 
  // The following are only used for non-allocated blocks. The first
  // pointer is also used as the pointer to allocated data in
  // allocated blocks (using a conversion).
  struct MemoryBlockHeader *prev ;
  struct MemoryBlockHeader *next ;
} ;

// Given a pointer to a memory block header, set the 
// correct address of the pointer to its start. These
// functions are not meant to be seen from outside 
// this file.
static inline void
SetStartPointer(struct MemoryBlockHeader*ptr) {
  uint32_t sp_ptr = 
    ((uint32_t)ptr) + chunks2bytes(ptr->al.length) 
    - sizeof(struct MemoryBlockHeader*) ;
  *(struct MemoryBlockHeader**)sp_ptr = ptr ;
}
static inline struct MemoryBlockHeader*
GetStartPointer(struct MemoryBlockHeader*ptr) {
  uint32_t sp_ptr = 
    ((uint32_t)ptr) + chunks2bytes(ptr->al.length) - sizeof(uint32_t) ;
  return *(struct MemoryBlockHeader**)sp_ptr ;
}
// Given a pointer to a memory block header, check that the
// address of the pointer to its start is correct.
static inline int
CheckStartPointer(struct MemoryBlockHeader* ptr) {
  return GetStartPointer(ptr) == ptr ;
}

// The heap base and the heap limit. The heap
// limit is no longer in the heap itself, it's
// the first address after it.
//
// Both variables are NULL for an uninitialized 
// heap. In this case malloc and free operations
// fail.
// 
static struct MemoryBlockHeader* heap_base = NULL ;
static struct MemoryBlockHeader* heap_limit = NULL ; 
static struct MemoryBlockHeader* free_mblock_list = NULL ;

// Attention: all these sizes are given in 
// 4-word chunks, because the length of all blocks
// is a multiple of it. At the same time, sizes 
// provided in parameter to the init and malloc functions 
// are in bytes (to comply with malloc conventions).
//
// The heap should be at least 16kbytes=1024 chunks
#define MIN_HEAP_SIZE_CHUNKS  0x400
// For safety, I set a max size on the size of an memory 
// block, which limits both the amount of RAM I can allocate
// to a heap and the amount I can malloc (because the initial
// empty heap has a single block).
// I set this to 2Mchunks=32Mbytes.
#define MAX_MBLOCK_SIZE_CHUNKS 0x200000

// Initialize the heap using a base address and a size,
// given in bytes. Attention, this means that a conversion
// to words is needed for internal accounting.
void init_malloc(char* base_address, 
		 size_t size) {
  {
    // First, a lot of sanity checks to ensure I don't do
    // stupid things afterwards. Remember I want to keep 
    // everything simple and safe.
    if((heap_base!=NULL)||(heap_limit!=NULL)) { 
      fatal_error("init_malloc: attempting init of already "
		  "initialized heap. Exiting...\n") ;
    }
    if(size < chunks2bytes(MIN_HEAP_SIZE_CHUNKS)) {
      fatal_error("init_malloc: attempting init of too small "
		  "a heap. Exiting...\n") ;
    }
    if(size > chunks2bytes(MAX_MBLOCK_SIZE_CHUNKS)) {
      fatal_error("init_malloc: heap size greater than "
		  "MAX_MBLOCK_SIZE_WORDS. Exiting...\n") ;
    }
    if((uint32_t)base_address<(uint32_t)&_end) {
      fatal_error("init_malloc: attempting init over program area. "
		  "Exiting...\n") ;
    }
    if((uint32_t)base_address>=ARM_RAM_SIZE) {
      fatal_error("init_malloc: attempting init beyond RAM limit. "
		  "Exiting...\n") ;
    }
    if(ARM_RAM_SIZE-(uint32_t)base_address < size) {
      fatal_error("init_malloc: attempting init partially beyond RAM limit. "
		  "Exiting...\n") ;
    }
    if(chunks2bytes(bytes2chunks((uint32_t)base_address))!=
       (uint32_t)base_address) {
      fatal_error("init_malloc: base_address not aligned. "
		  "Exiting...\n") ;
    }
    if(chunks2bytes(bytes2chunks(size))!=size) {
      fatal_error("init_malloc: heap size not aligned. "
		  "Exiting...\n") ;
    }
  }
  // Finally, I can init the heap. The address of heap_limit should
  // never be dereferenced, because it may be outside the accessible
  // memory.
  heap_base  = (struct MemoryBlockHeader*) base_address ;
  heap_limit = (struct MemoryBlockHeader*) (((uint32_t)heap_base) + size) ;
  // Initializing the list of free blocks to contain a 
  // single block containing all the memory.  
  free_mblock_list = heap_base ;
  free_mblock_list->al.length = bytes2chunks(size) ;
  free_mblock_list->al.allocated = 0 ;
  free_mblock_list->prev = NULL ;
  free_mblock_list->next = NULL ;
  SetStartPointer(free_mblock_list) ;
}

// This is a debug function that should function even if
// malloc does not. It does not use debug_printf (who
// uses va_arg and possibly other functions that use 
// malloc/free.  This function only uses debug_puts, which
// statically allocates stuff on the stack.
void DebugPrintHeapOrganization() {
  debug_puts("DebugPrintHeapOrganization:\n") ;
  // Check if the heap was initialized.
  if((heap_base==NULL)||
     (heap_limit==NULL)) { 
    debug_puts("The heap was not initialized.\n") ;
  } else {
    char buf[128] ;
    debug_puts("\theap_base:") ;
    uint32ascii((uint32_t)heap_base,16,8,10,buf) ;
    debug_puts(buf) ;
    debug_puts("\theap_limit:") ;
    uint32ascii((uint32_t)heap_limit,16,8,10,buf) ;
    debug_puts(buf) ;
    debug_puts("\tfree_block_list:") ;
    uint32ascii((uint32_t)free_mblock_list,16,8,16,buf) ;
    debug_puts(buf) ;
    debug_puts("\tstack:") ;
    {
      // This may not be as precise as calling
      // get_stack_pointer(), but it simplifies
      // code organization.
      volatile uint32_t SP ;
      asm ("mov %[res], sp ":[res]"=r"(SP)) ;
      uint32ascii(SP,16,8,10,buf) ;
    }
    debug_puts(buf) ;
    debug_puts("\n") ;
    struct MemoryBlockHeader* tmp_mblock = heap_base ;
    while(tmp_mblock < heap_limit) {
      debug_puts("\t") ;
      uint32ascii((uint32_t)tmp_mblock,16,8,10,buf) ;
      debug_puts(buf) ;
      debug_puts(": LEN=") ;
      uint32ascii(chunks2bytes(tmp_mblock->al.length),16,8,10,buf) ;
      debug_puts(buf) ;
      debug_puts(": STARTP=") ;
      uint32ascii((uint32_t)GetStartPointer(tmp_mblock),16,8,10,buf) ;
      debug_puts(buf) ;
      if(tmp_mblock->al.allocated) {
	debug_puts(" ALLOCATED") ;
      } else {
	debug_puts(" FREE: PREV:") ;
	uint32ascii((uint32_t)tmp_mblock->prev,16,8,10,buf) ;
	debug_puts(buf) ;
	debug_puts(" NEXT:") ;
	uint32ascii((uint32_t)tmp_mblock->next,16,8,10,buf) ;
	debug_puts(buf) ;
      }
      debug_puts("\n") ;
      tmp_mblock = 
	(struct MemoryBlockHeader*)
	(((uint32_t)tmp_mblock) + 
	 chunks2bytes(tmp_mblock->al.length)) ;
    }
  }
}


// Check if two free memory blocks are adjacent,
// which allows them to be merged.
static inline int 
IsAdjacent(const struct MemoryBlockHeader*b1,
	   const struct MemoryBlockHeader*b2) {
  return ((uint32_t)b1)+chunks2bytes(b1->al.length) == (uint32_t)b2 ; 
}

// Returns 1 if a merge has been performed, 0 otherwise.
// In case a merge is performed, the first block is 
// preserved and increases in size.
static inline int 
MergeBlocksIfPossible(struct MemoryBlockHeader*b1,
		      struct MemoryBlockHeader*b2) {
  if((b1==NULL)||(b2==NULL)) {
    fatal_error("free:MergeBlocksIfPossible: cannot accept NULL pointers. "
		"Exiting...\n") ;
  }
  if((b1->al.allocated)||(b2->al.allocated)) {
    fatal_error("free:MergeBlocksIfPossible: cannot accept allocated blocks. "
		"Exiting...\n") ;
  }
  if(IsAdjacent(b1,b2)) {
    // First, change the length
    b1->al.length += b2->al.length ;
    // Next, update the start pointer
    SetStartPointer(b1) ;
    // Change the pointers in the list
    b1->next = b2->next ;
    if(b2->next != NULL) {
      b2->next->prev = b1 ;
    }
    return 1 ;
  } else {
    return 0 ;
  }
}

/* Memory free function.
   It only works correctly when called with a 
   pointer provided by malloc and not yet freed.
   There are some checks to ensure this, but
   can be fooled.
 */
void free(void *ptr) {
  /*
  char dprint_buf[256] ;
  debug_puts("free: called with ptr=") ;
  uint32ascii((uint32_t)ptr,16,dprint_buf,8) ;
  debug_puts(dprint_buf) ;
  debug_puts("and memory state:") ;
  DebugPrintHeapOrganization() ;
  */
  // Check if the heap was initialized.
  if((heap_base==NULL)||(heap_limit==NULL)) { 
    fatal_error("free: the heap was not initialized. "
		"Exiting...\n") ;
  } 
  // We assume that the pointer provided in argument 
  // points to the second word of an allocated memory 
  // block (the first word of data), cf. the definitions
  // above.
  // To obtain the pointer to the memory block I have to 
  // skip back one word.
  struct MemoryBlockHeader* mblock_to_free = 
    (struct MemoryBlockHeader*)(((uint32_t*)ptr)-1) ;
  {
    // Sanity checks to detect obvious memory corruption.
    // Any error here leads to immediate termination
    // of execution.

    // Check if ptr points inside the heap.
    if((mblock_to_free < heap_base)||
       (mblock_to_free >= heap_limit)) {
      fatal_error("free: pointer is outside the heap. Exiting...\n") ;
    }
    // I consider that freeing memory that was not
    // allocated is an error.
    if(!mblock_to_free->al.allocated) {
      // Memory is corrupted because this block which I 
      // call free upon is not allocated.
      fatal_error("free: freeing non-allocated block or "
		  "memory corruption. Exiting...\n") ;
    }
    // Check consistency of the pointers.
    if(!CheckStartPointer(mblock_to_free)) {
      // Memory is corrupted because my block does not have
      // the final pointer set right.
      fatal_error("free: corrupt memory block found(1). Exiting...\n") ;
    }
  }
  // I navigate back through allocated blocks until I 
  // reach a non-allocated one or the heap_base.
  struct MemoryBlockHeader* tmp_mblock = mblock_to_free ;
  do {
    if(tmp_mblock == heap_base) {
      // I have finished searching. Now I know that 
      // the block I free will become the first in the 
      // free block list, because there is no free block
      // before it.
      mblock_to_free->al.allocated = 0 ;
      mblock_to_free->next = free_mblock_list ;
      mblock_to_free->prev = NULL ;
      free_mblock_list = mblock_to_free ;
      // Now, I must attempt to merge the new free
      // block with the one after it, if there is 
      // one.
      if(mblock_to_free->next != NULL) {
	mblock_to_free->next->prev = mblock_to_free ;
	MergeBlocksIfPossible(tmp_mblock,
			      mblock_to_free->next) ;
      }
      // free is now complete in this case.
      /*
      debug_puts("free: terminated with ") ;
      debug_puts("memory state:\n") ;
      DebugPrintHeapOrganization() ;
      */
      return ;
    } else {
      // I still can move to previous blocks.
      // I do this by reading the "start" pointer of
      // the previous block, placed in the word before
      // tmp_mblock.
      tmp_mblock = 
	(struct MemoryBlockHeader*)(((uint32_t**)tmp_mblock)[-1]) ;
      // Before anything else, perform the sanity checks
      // on this block.
      if(!CheckStartPointer(tmp_mblock)) {
	// Memory is corrupted, as the block does not have
	// the start pointer set right with respect to its 
	// length. I abandon execution.
	fatal_error("free: corrupt memory block found(2). Exiting...\n") ;
      }
    }
  } while (tmp_mblock->al.allocated) ;
  // Now, tmp_block is a non-allocated (free) block.
  // I will need to insert mblock_to_free 
  // after tmp_block (and maybe perform some free block
  // merges).
  // First, change the type of the block I need to free.
  mblock_to_free->al.allocated = 0 ;
  // Next, insert it the linked list.
  mblock_to_free->next = tmp_mblock->next ;
  tmp_mblock->next = mblock_to_free ;
  mblock_to_free->prev = tmp_mblock ;
  if(mblock_to_free->next != NULL) {
    mblock_to_free->next->prev = mblock_to_free ;
  }
  // The linked list is now complete. However, one operation
  // remains to be done: merging of adjacent free memory 
  // blocks. In the worst case, mblock_to_free becoming
  // free requires its merging with both the free block
  // before it, and the one after it. Of course, it can 
  // also happen that no merge is needed.
  //
  // Since the MergeBlocks function preserves its first 
  // argument, I have to perform the operations in the
  // following order:
  if(mblock_to_free->next != NULL) {
    MergeBlocksIfPossible(mblock_to_free,
			  mblock_to_free->next) ;
  }
  MergeBlocksIfPossible(tmp_mblock,mblock_to_free) ;
  /*
  debug_puts("free: terminated with ") ;
  debug_puts("memory state:\n") ;
  DebugPrintHeapOrganization() ;
  */
}

void* malloc(size_t size) {
  /*
  char dprint_buf[256] ;
  debug_puts("malloc: called with size=") ;
  uint32ascii(size,10,dprint_buf,8) ;
  debug_puts(dprint_buf) ;
  debug_puts("and memory state:") ;
  DebugPrintHeapOrganization() ;
  */

  // Check if the heap was initialized.
  if((heap_base==NULL)||
     (heap_limit==NULL)) { 
    fatal_error("malloc: the heap was not initialized.") ;
  }
  // No need to check for upper size limit, it is implicitly
  // done through the search for free intervals. However,
  // I will not accept reservation requests with size 0
  // (I will simply return NULL).
  if(size == 0 ) {
    warning("malloc: request size 0. return NULL.\n") ;
    errno = EINVAL ;
    return NULL ;
  }
  // The size parameter does not account for the header
  // and the final start pointer, so I need to account 
  // for them
  size += 2* sizeof(uint32_t) ;
  // The size parameter is provided in bytes, whereas
  // all allocation is done in 4-word chunks. I need to
  // align.
  if(chunks2bytes(bytes2chunks(size))!=size) {
    size = chunks2bytes(bytes2chunks(size)+1) ;
  }

  // Comment the following #define if one does not need
  // to isolate allocated blocks with non-allocated ones.
#define ISOLATE_ALLOCATED
#ifdef ISOLATE_ALLOCATED
  // To ensure that I never have 2 consecutive allocated 
  // blocks, I need to be sure that I can leave free
  // blocks before and after the reservation.
  // A simple condition allowing this is that the size is
  // increased by the size of two minimal free blocks.
  size += chunks2bytes(2) ;
#endif

  // Now, the size is the good one, and I will start my search 
  // through the free interval list until I find one 
  // large-enough block (first fit). I could replace with a 
  // next fit or a best fit algorithm, if needed.
  struct MemoryBlockHeader* tmp_mblock = free_mblock_list ;
  while((tmp_mblock != NULL) &&
	(chunks2bytes(tmp_mblock->al.length)<size)) {
    tmp_mblock = tmp_mblock->next ;
  }
  if(tmp_mblock == NULL) {
    // Malloc did not find a large enough block (this can 
    // mean two things: memory is fragmented or there is 
    // simply no memory left).
    warning("malloc: out of memory. heap state:") ;
    DebugPrintHeapOrganization() ;
    errno = ENOMEM ;
    return NULL ;
  } 
  // tmp_mblock->al.length>=size, so I can (and will)
  // reserve in this memory block.
  //
#ifdef ISOLATE_ALLOCATED
  // As I'm doing here the real allocation, I need to reduce to the 
  // real size I need.
  size -= chunks2bytes(2) ;
  // First, separate one minimal chunk and let it non-allocated
  // to ensure isolation.
  {
    struct MemoryBlockHeader* split_mblock = 
      (struct MemoryBlockHeader*)(((uint32_t)tmp_mblock) + chunks2bytes(1)) ;
    split_mblock->al.allocated = 0 ;
    split_mblock->al.length = tmp_mblock->al.length - 1 ; // counting in chunks
    split_mblock->next = tmp_mblock->next ;
    split_mblock->prev = tmp_mblock ;
    if(split_mblock->next != NULL) {
      split_mblock->next->prev = split_mblock ;
    }
    tmp_mblock->al.length = 1 ;
    SetStartPointer(split_mblock) ;
    SetStartPointer(tmp_mblock) ;
    // It's the second that will be split next.
    tmp_mblock = split_mblock ;
  }
  //DebugPrintHeapOrganization() ;
#endif
  // Now, I check if I have to split the remaining
  // block (in case ISOLATED_ALLOCATE is set, this
  // is always the case).
  if(bytes2chunks(size)<tmp_mblock->al.length) {
    // The free memory block tmp_mblock is larger than what I
    // need. I will split it in two. The first part I reserve, 
    // and the second I leave as a free block.
    struct MemoryBlockHeader* split_mblock = 
      (struct MemoryBlockHeader*)(((uint32_t)tmp_mblock) + size) ;
    split_mblock->al.allocated = 0 ;
    split_mblock->al.length = tmp_mblock->al.length - bytes2chunks(size) ;
    split_mblock->next = tmp_mblock->next ;
    split_mblock->prev = tmp_mblock ;
    tmp_mblock->next = split_mblock ;
    if(split_mblock->next != NULL) {
      split_mblock->next->prev = split_mblock ;
    }
    tmp_mblock->al.length = bytes2chunks(size) ;
    SetStartPointer(split_mblock) ;
    SetStartPointer(tmp_mblock) ;
  }
  //DebugPrintHeapOrganization() ;
  // At this point, tmp_mblock has exactly the size
  // we need to reserve. I will simply reserve the 
  // whole tmp_mblock for my data and return 
  // a pointer.
  tmp_mblock->al.allocated = 1 ;
  struct MemoryBlockHeader* prev_mblock = tmp_mblock->prev ;
  struct MemoryBlockHeader* next_mblock = tmp_mblock->next ;
  if(prev_mblock == NULL) {
    // We reserved the first block in the list 
    free_mblock_list = next_mblock ;
  } else {
    prev_mblock->next = next_mblock ;
  }
  if(next_mblock != NULL) {
    next_mblock->prev = prev_mblock ;
  }
  // I finished, now I return a pointer to the data 
  // area, which has the same address as tmp_mblock->prev.
  errno = ENONE ;
  return (void*)&(tmp_mblock->prev) ;
}

// The posix_memalign() function allocates size bytes of memory such
// that the allocation's base address is an exact multiple of
// alignment, and returns the allocation in the value pointed to by
// memptr. The requested alignment must be a power of 2 at least as
// large as sizeof(void *).
//
// The posix_memalign() function returns the value 0 if successful;
// otherwise it returns an error value.
// 
// Due to the implementation constraints, pointers returned by 
// posix_memalign cannot be freed. This is OK for now, because only
// the kernel uses it, for the scheduler data structures.
int32_t posix_memalign(void **memptr, size_t alignment, size_t size){
  // Check if the requested alignment is correct.
  {
    size_t tmp = alignment ;
    while(tmp && !(tmp&0x1)) {
      tmp >>= 1 ;
    }
    if((alignment < sizeof(void*))||
       (tmp != (tmp&0x1))) {
      *memptr = NULL ; 
      errno = EINVAL ;
      return errno ;
    }
  }
  // To allow allocation with the desired alignment,
  // I make provision for an extra alignment-1 bytes.
  size += (alignment-1) ;
  // malloc the whole thing
  char* alloc = malloc(size) ;
  // Align, the simple way, taking advantage of the 
  // fact that alignment is a power of 2.
  char* result = alloc ;
  alignment -- ;
  while(alignment&(uint32_t)result) result++ ;
  // At this point, the pointer should be aligned and it 
  // should not have increased by more than alignment
  // bytes. Attention, alignment has been decremented to
  // improve speed.
  if(result-alloc > alignment) {
    fatal_error("posix_memalign: cannot align.") ;
  }
  // I finished, now I return.
  errno = ENONE ;
  *memptr = (void*)result ;
  return errno ;
}
