
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
#include <libc/errno.h>  // For error codes
#include <libc/stdlib.h> // For malloc/free
#include <libc/string.h> // For strings
#include <libc/util.h>   // For unaligned accesses
#include <libc/arm-eabi.h> // For div/mod
#include <librpi/debug.h>
#include <librpi/uart.h>
#include <libsdfs/fs.h>    

//#define fat_debug_printf(...) debug_printf(__VA_ARGS__)
#define fat_debug_printf(...)



struct fat_fs {
	struct fs b;
	int fat_type;
	uint32_t total_sectors;
	uint32_t sectors_per_cluster;
	uint32_t bytes_per_sector;
	char *vol_label;
	uint32_t first_fat_sector;
	uint32_t first_data_sector;
	uint32_t sectors_per_fat;
	uint32_t root_dir_entries;
	uint32_t root_dir_sectors;
	uint32_t first_non_root_sector;
	uint32_t root_dir_cluster;
};

// FAT32 extended fields
struct fat_extBS_32
{
	uint32_t		table_size_32;
	uint16_t		extended_flags;
	uint16_t		fat_version;
	uint32_t		root_cluster;
	uint16_t		fat_info;
	uint16_t		backup_BS_sector;
	uint8_t			reserved_0[12];
	uint8_t			drive_number;
	uint8_t			reserved_1;
	uint8_t			boot_signature;
	uint32_t		volume_id;
	char			volume_label[11];
	uint8_t			fat_type_label[8];
} __attribute__ ((packed));

// FAT 12/16 extended fields
struct fat_extBS_16
{
	uint8_t			bios_drive_num;
	uint8_t			reserved1;
	uint8_t			boot_signature;
	uint32_t		volume_id;
	char			volume_label[11];
	uint8_t			fat_type_label[8];
} __attribute__ ((packed));


// Generic FAT fields
struct fat_BS
{
	uint8_t			bootjmp[3];
	uint8_t			oem_name[8];
	uint16_t		bytes_per_sector;
	uint8_t			sectors_per_cluster;
	uint16_t		reserved_sector_count;
	uint8_t			table_count;
	uint16_t		root_entry_count;
	uint16_t		total_sectors_16;
	uint8_t			media_type;
	uint16_t		table_size_16;
	uint16_t		sectors_per_track;
	uint16_t		head_side_count;
	uint32_t		hidden_sector_count;
	uint32_t		total_sectors_32;

	union
	{
		struct fat_extBS_32	fat32;
		struct fat_extBS_16	fat16;
	} ext;
} __attribute__ ((packed));

struct TestStruct{
  uint8_t  c ;
  uint16_t s ;
  uint32_t u ;
}  __attribute__ ((packed));


enum FATNames {
  FAT12 = 0 ,
  FAT16 = 1 ,
  FAT32 = 2 ,
  VFAT  = 3 
} ;

static const char *fat_names[] = { "FAT12", "FAT16", "FAT32", "VFAT" };


static struct dirent *fat_read_dir(struct fat_fs *fs, struct dirent *d);
struct dirent *fat_read_directory(struct fs *fs, char **name);
static uint32_t fat_get_next_bdev_block_num(uint32_t f_block_idx, FILE *s, 
					    void *opaque, int add_blocks);

struct fat_file_block_offset
{
	uint32_t f_block;
	uint32_t cluster;
};


static FILE *fat_fopen(struct fs *fs, struct dirent *path, const char *mode)
{
	if(fs != path->fs)
	{
		errno = EFAULT;
		return (FILE *)0;
	}

	if(strcmp(mode, "r"))
	{
		errno = EROFS;
		return (FILE *)0;
	}

	struct vfs_file *ret = (struct vfs_file *)malloc(sizeof(struct vfs_file));
	memset(ret, 0, sizeof(struct vfs_file));
	ret->fs = fs;
	ret->pos = 0;
	ret->opaque = path->opaque;
	ret->len = (long)path->byte_size;

	(void)mode;
	return ret;
}

static size_t fat_fread(struct fs *fs, void *ptr, size_t byte_size, FILE *stream)
{
  fat_debug_printf("XXXXXXXXX Entered fat.c::fat_fread\n") ;

  if(stream->fs != fs)
    return -1;
  if(stream->opaque == (void *)0)
    return -1;
  
  struct fat_file_block_offset opaque;
  opaque.cluster = (uint32_t)stream->opaque;
  opaque.f_block = 0;
  return fs_fread(fat_get_next_bdev_block_num, fs, 
		  ptr, byte_size, stream, (void*)&opaque);
}

static int fat_fclose(struct fs *fs, FILE *fp)
{
	(void)fs;
	(void)fp;
	return 0;
}

int fat_init(struct block_device *parent, struct fs **fs) {
  // Interpret a FAT file system
  fat_debug_printf("FAT: looking for a filesytem on %s\n", 
	       parent->device_name);
  
  // Read block 0
  uint8_t *block_0 = (uint8_t *)malloc(512);
  int r = block_read(parent, block_0, 512, 0);
  if(r < 0) {
    fat_debug_printf("FAT: error %d reading block 0\n", r);
    return r;
  }
  if(r != 512) {
    fat_debug_printf("FAT: error reading block 0 (only %d bytes read)\n", r);
    return -1;
  }
  
  // Dump the boot block
  fat_debug_printf("Dump the boot block:\n");
  int j = 0;
  for(int i = 0; i < 90; i++) {
    fat_debug_printf("%2x ", block_0[i]);
    j++; 
    if(j == 8) {
      j = 0;
      fat_debug_printf("\n");
    } 
  }
  if(j != 0) {
    fat_debug_printf("\n");
  }

  struct fat_BS *bs = (struct fat_BS *)block_0;
  fat_debug_printf("Decoding the boot sector: \n"
	       "\tLead bytes: %2x %2x %2x\n"
	       "\tOEM name: %s\n"
	       "\tBytes per sector: %x\n"
	       "\tSectors per cluster: %x\n"
	       "\tReserved sector count: %x\n"
	       "\tTable count: %x\n"
	       "\tRoot entry count: %x\n"
	       "\tTotal sectors 16: %x\n"
	       "\tMedia type: %x\n"
	       "\tTable size 16: %x\n"
	       "\tSectors per track: %x\n"
	       "\tHead side count: %x\n"
	       "\tHidden sector count: %x\n"
	       "\tTotal sectors 32: %x\n"
	       ,
	       bs->bootjmp[0], bs->bootjmp[1], bs->bootjmp[2],
	       bs->oem_name,
	       bs->bytes_per_sector,
	       bs->sectors_per_cluster,
	       bs->reserved_sector_count,
	       bs->table_count,
	       bs->root_entry_count,
	       bs->total_sectors_16,
	       bs->media_type,
	       bs->table_size_16,
	       bs->sectors_per_track,
	       bs->head_side_count,
	       bs->hidden_sector_count,
	       bs->total_sectors_32) ;

  if(bs->bootjmp[0] != 0xeb) {
    fat_debug_printf("FAT: not a valid FAT filesystem on %s (%x)\n", 
		 parent->device_name,
		 bs->bootjmp[0]);
    return -1;
  }
  
  uint32_t total_sectors = bs->total_sectors_16;
  if(total_sectors == 0)
    total_sectors = bs->total_sectors_32;
  fat_debug_printf("FAT: total_sectors=%x\n",total_sectors) ;

  struct fat_fs *ret = (struct fat_fs *)malloc(sizeof(struct fat_fs));
  memset(ret, 0, sizeof(struct fat_fs));
  ret->b.fopen = fat_fopen;
  ret->b.fread = fat_fread;
  ret->b.fclose = fat_fclose;
  ret->b.read_directory = fat_read_directory;
  ret->b.parent = parent;
  
  ret->total_sectors = total_sectors;
  {
    uint32_t buf[16] ;
    struct TestStruct* str = (struct TestStruct*)buf ;
    str->s = 0x200 ;
    uint16_t b ;
    uint32_t c ;
    b=str->s ;
    c=b ;
    fat_debug_printf("FAT:TEST: struct(%x) a(%x)=%x b(%x)=%x c(%x)=%x\n",
		 &(str->c),
		 (uint32_t)&(str->s),str->s,
		 (uint32_t)&b,b,
		 (uint32_t)&c,c) ;
    
    uint16_t tmp = bs->bytes_per_sector ;
    ret->bytes_per_sector = (uint32_t)tmp ;
    fat_debug_printf("FAT: ret->bytes_per_sector=%x bs->bytes_per_sector=%x tmp=%x\n",
		 (uint32_t)ret->bytes_per_sector,
		 (uint32_t)bs->bytes_per_sector,
		 (uint32_t)tmp) ;

    {
      char* p ;
      char* q ;
      p = (char*)&(bs->bytes_per_sector) ;
      q = (char*)&(ret->bytes_per_sector) ;
      fat_debug_printf("\tbs->bytes_per_sector: %2x %2x\n"
		   "\tret->bytes_per_sector: %2x %2x %2x %2x\n",
		   p[0],p[1],
		   q[0],q[1],q[2],q[3]) ;
    }
  }
  ret->root_dir_entries = bs->root_entry_count;
  ret->root_dir_sectors = 
    // The + bytes_per_sector - 1 rounds up the sector no
    rpi_uidiv((ret->root_dir_entries * 32 + ret->bytes_per_sector - 1),
	      ret->bytes_per_sector);	
  
  uint32_t fat_size = bs->table_size_16;
  if(fat_size == 0)
    fat_size = bs->ext.fat32.table_size_32;
  
  uint32_t data_sec = total_sectors - (bs->reserved_sector_count + 
				       bs->table_count * fat_size + 
				       ret->root_dir_sectors);
  
  uint32_t total_clusters = rpi_uidiv(data_sec, bs->sectors_per_cluster);
  if(total_clusters < 4085) {
    fat_debug_printf("FAT12 system identified\n") ;
    ret->fat_type = FAT12;
  } else if(total_clusters < 65525) {
    fat_debug_printf("FAT16 system identified\n") ;
    ret->fat_type = FAT16;
  } else {
    ret->fat_type = FAT32;
    fat_debug_printf("FAT32 system identified\n") ;
  }
  ret->b.fs_name = fat_names[ret->fat_type];
  fat_debug_printf("File system name: %u %s\n",ret->fat_type,ret->b.fs_name) ;
  ret->sectors_per_cluster = bs->sectors_per_cluster;
  fat_debug_printf("Sectors per cluster: %x\n",ret->sectors_per_cluster) ;
  
  // XXX
  
  fat_debug_printf("FAT: reading a %s filesystem: total_sectors %d, "
	       "sectors_per_cluster %d, "
	       "bytes_per_sector %d\n",
	       ret->b.fs_name, ret->total_sectors, 
	       ret->sectors_per_cluster,
	       ret->bytes_per_sector);
  
  // Interpret the extended bpb
  ret->vol_label = (char *)malloc(12);
  fat_debug_printf("after malloc(12):\n") ;
  //DebugPrintHeapOrganization() ;

  if(ret->fat_type == FAT32) {
    // FAT32
    strncpy(ret->vol_label, bs->ext.fat32.volume_label,11);
    ret->vol_label[11] = 0;
    fat_debug_printf("FAT32: volume label: %s\n", ret->vol_label);
    
    ret->first_data_sector = bs->reserved_sector_count + (bs->table_count *
							  bs->ext.fat32.table_size_32);
    ret->first_fat_sector = bs->reserved_sector_count;
    ret->first_non_root_sector = ret->first_data_sector;
    ret->sectors_per_fat = bs->ext.fat32.table_size_32;
    
    fat_debug_printf("FAT: first_data_sector: %d, first_fat_sector: "
		 "%d\n",
		 ret->first_data_sector,
		 ret->first_fat_sector);
    
    ret->root_dir_cluster = bs->ext.fat32.root_cluster;
  } else {	
    // FAT12/16
    strncpy(ret->vol_label, bs->ext.fat16.volume_label,11);
    ret->vol_label[11] = '\0';
    fat_debug_printf("after strncpy:\n") ;
    //DebugPrintHeapOrganization() ;

    
    fat_debug_printf("FAT12/16: volume label: %s\n", ret->vol_label);
    
    
    ret->first_data_sector = bs->reserved_sector_count + (bs->table_count *
							  bs->table_size_16);
    ret->first_fat_sector = bs->reserved_sector_count;
    ret->sectors_per_fat = bs->table_size_16;
    
    fat_debug_printf("FAT: first_data_sector: %d, "
		 "first_fat_sector: %d\n",
		 ret->first_data_sector,
		 ret->first_fat_sector);
    
    fat_debug_printf("FAT: root_dir_entries: %d, "
		 "root_dir_sectors: %d\n",
		 ret->root_dir_entries,
		 ret->root_dir_sectors);
    
    ret->first_non_root_sector = ret->first_data_sector + ret->root_dir_sectors;
    ret->root_dir_cluster = 2;
  }
  
  ret->b.block_size = ret->bytes_per_sector * ret->sectors_per_cluster;
  *fs = (struct fs *)ret;
  safe_free(block_0);
  
  fat_debug_printf("FAT: found a %s filesystem on %s\n", 
	       ret->b.fs_name, ret->b.parent->device_name);
  
  return 0;
}

uint32_t get_sector(struct fat_fs *fs, uint32_t rel_cluster) {
  fat_debug_printf("FAT: get_sector rel_cluster %d, sector %d\n",
		   rel_cluster,
		   fs->first_non_root_sector + 
		   (rel_cluster - 2) * fs->sectors_per_cluster);
  rel_cluster -= 2;
  return fs->first_non_root_sector + rel_cluster * fs->sectors_per_cluster;
}

static uint32_t get_next_fat_entry(struct fat_fs *fs, 
				   uint32_t current_cluster) {
	switch(fs->fat_type)
	{
		case FAT16:
			{
				uint32_t fat_offset = current_cluster << 1; // *2
				uint32_t fat_sector = 
				  fs->first_fat_sector +
				  rpi_uidiv(fat_offset, fs->bytes_per_sector);
				uint8_t *buf = (uint8_t *)malloc(512);
				int br_ret = block_read(fs->b.parent, buf, 512, fat_sector);
				if(br_ret < 0)
				{
					fat_debug_printf("FAT: block_read returned %d\n");
					return 0x0ffffff7;
				}
				uint32_t fat_index = rpi_uimod(fat_offset, fs->bytes_per_sector);
				// JP: the following line is very bizarre. I have changed
				//     it using the functions in util.h.
				//uint32_t next_cluster = (uint32_t)*(uint16_t *)&buf[fat_index];
				uint32_t next_cluster = (uint32_t)read_halfword(buf,fat_index) ;
				safe_free(buf);
				if(next_cluster >= 0xfff7)
				  next_cluster |= 0x0fff0000;
				return next_cluster;
			}

		case FAT32:
			{
				uint32_t fat_offset = current_cluster << 2; // *4
				uint32_t fat_sector = fs->first_fat_sector +
				  rpi_uidiv(fat_offset, fs->bytes_per_sector);
				uint8_t *buf = (uint8_t *)malloc(512);
				int br_ret = block_read(fs->b.parent, buf, 512, fat_sector);
				if(br_ret < 0)
				{
					fat_debug_printf("FAT: block_read returned %d\n");
					return 0x0ffffff7;
				}
				uint32_t fat_index = rpi_uimod(fat_offset, fs->bytes_per_sector);

				// JP: the following line is very bizarre. I have changed
				//     it using the functions in util.h.
				//uint32_t next_cluster = *(uint32_t *)&buf[fat_index];
				uint32_t next_cluster = read_word(buf,fat_index) ;
				safe_free(buf);
				return next_cluster & 0x0fffffff; // FAT32 is actually FAT28
			}
		default:
			fat_debug_printf("FAT: fat type %s not supported\n", fs->b.fs_name);
			return 0;
	}
}

struct dirent *fat_read_directory(struct fs *fs, char **name)
{
	struct dirent *cur_dir = fat_read_dir((struct fat_fs *)fs, (void*)0);
	while(*name)
	{
		// Search the directory entries for one of the requested name
		int found = 0;
		while(cur_dir)
		{
			if(!strcmp(*name, cur_dir->name))
			{
				if(!cur_dir->is_dir)
				{
					errno = ENOTDIR;
					return (void*)0;
				}
				found = 1;
				cur_dir = fat_read_dir((struct fat_fs *)fs, cur_dir);
				name++;
				break;
			}
			cur_dir = cur_dir->next;
		}
		if(!found)
		{
		  fat_debug_printf("FAT: path part %s not found\n", *name);
		  errno = ENOENT;
		  return (void*)0;
		}
	}
	return cur_dir;
}

static uint32_t fat_get_next_bdev_block_num(uint32_t f_block_idx, 
					    FILE *s, 
					    void *opaque, 
					    int add_blocks) {
  struct fat_file_block_offset *ffbo = (struct fat_file_block_offset *)opaque;
  
  // Iterate through the cluster chain until we reach the appropriate one
  while((ffbo->f_block != f_block_idx) && (ffbo->cluster < 0x0ffffff8)) {
    ffbo->cluster = get_next_fat_entry((struct fat_fs *)s->fs, ffbo->cluster);
    ffbo->f_block++;
  }
  
  if(ffbo->cluster < 0x0ffffff8)
    return get_sector((struct fat_fs *)s->fs, ffbo->cluster);
  else {
    if(add_blocks) {
      fat_debug_printf("FAT: request to extend cluster chain not currently supported\n");
    }
    s->flags |= VFS_FLAGS_EOF;
    return 0xffffffff;
  }
}

struct dirent *fat_read_dir(struct fat_fs *fs, struct dirent *d)
{
  int is_root = 0;
  struct fat_fs *fat = (struct fat_fs *)fs;
  
  if(d == NULL) {
    is_root = 1;
  }
  uint32_t cur_cluster;
  uint32_t cur_root_cluster_offset = 0;
  if(is_root) {
    cur_cluster = fat->root_dir_cluster;
  } else {
    cur_cluster = (uint32_t)d->opaque;
  }

  struct dirent *ret = NULL ;
  struct dirent *prev = NULL ;

  fat_debug_printf("FAT: read_dir: starting directory read "
		   "from cluster %d\n", cur_cluster);

  {
    // Reserve place on the stack for reading a cluster
    // from the SD card.
    uint32_t cluster_size = 
      fat->bytes_per_sector * fat->sectors_per_cluster;
    uint8_t buf[cluster_size] ;

    do {
      /* Interpret the cluster number to an absolute address */
      uint32_t absolute_cluster = cur_cluster - 2;
      uint32_t first_data_sector = fat->first_data_sector;
      if(!is_root) {
	first_data_sector = fat->first_non_root_sector;
      }		
      fat_debug_printf("FAT: reading cluster %d (sector %d)\n", 
		       cur_cluster,
		       absolute_cluster * fat->sectors_per_cluster + 
		       first_data_sector);
      
      int br_ret = block_read(fat->b.parent, 
			      buf, 
			      cluster_size, 
			      absolute_cluster * fat->sectors_per_cluster + first_data_sector);
      
      if(br_ret < 0) {
	fat_debug_printf("FAT: block_read returned %d\n", br_ret);
	return NULL;
      }
      
      // JP: The cluster is formed of 32-byte records
      for(uint32_t ptr = 0; ptr < cluster_size; ptr += 32) {
	// Does the entry exist (if the first byte is zero of 0xe5 it doesn't)
	if((buf[ptr] == 0) || (buf[ptr] == 0xe5)) continue;
	
	// Is it the directories '.' or '..'?
	if(buf[ptr] == '.') continue;
	
	// Is it a long filename entry (if so ignore, maybe TODO in the future)
	if(buf[ptr + 11] == 0x0f) continue;
	
	// Else read it
	// The malloc here is justified, it cannot be replaced with
	// stack allocation.
	struct dirent *de = (struct dirent *)malloc(sizeof(struct dirent));
	fat_debug_printf("Allocated dirent at address: %x\n",(uint32_t)de) ;
	memset(de, 0, sizeof(struct dirent));
	// Update the linked list of entries.
	{
	  if(ret == NULL) {
	    ret = de;
	  }
	  if(prev != NULL) {
	    prev->next = de;
	  } 
	  prev = de;
	  de->next = NULL ;
	}
	{
	  // Support only 8+3 filenames and convert them
	  // to lowercase. 
	  //	  de->name = (char *)malloc(13);
	  //fat_debug_printf("Allocated de->name at address: %x\n"
	  //		   "\tAddress of de->opaque: %x\n",
	  //		   (uint32_t)(de->name),
	  //		   (uint32_t)&(de->opaque)
	  //		   ) ;
	  //DebugPrintHeapOrganization() ;

	  de->fs = &fs->b;
	  // Convert to lowercase on load
	  int d_idx = 0 ;
	  fat_debug_printf("Reading a filename:") ;
	  for(int i = 0; (i < 8)&&(' '!=(char)buf[ptr+i]); i++) {
	    fat_debug_printf("%x ",buf[ptr+i]) ;
	    char cur_v = (char)buf[ptr+i];
	    if((cur_v >= 'A') && (cur_v <= 'Z')) {
	      cur_v -= 'A' ;
	      cur_v += 'a' ;
	    }
	    de->name[d_idx++] = cur_v;
	  }
	  de->name[d_idx++] = '.';
	  for(int i = 8; (i < 11)&&(' '!=(char)buf[ptr+i]); i++) {
	    fat_debug_printf("%x ",buf[ptr+i]) ;
	    char cur_v = (char)buf[ptr+i];
	    if((cur_v >= 'A') && (cur_v <= 'Z')) {
	      cur_v -= 'A' ;
	      cur_v += 'a' ;
	    }
	    de->name[d_idx++] = cur_v;
	  }
	  if(de->name[d_idx-1]=='.') {
	    de->name[d_idx-1] = '\0' ;
	  } else {
	    de->name[d_idx] = '\0' ;
	  }
	}

	if(buf[ptr + 11] & 0x10) {
	  de->is_dir = 1;
	}
	de->byte_size = read_word(buf, ptr + 28);

	// The following 2 lines define opaque as the "first cluster"
	// entry with FAT32 capabilities. In FAT32, the standard 2 bytes
	// "first cluster" need to be completed with another 2 bytes
	// from a previously reserved area (which is not contiguous with
	// the original "first cluster"). 
	// cf: http://www.cse.scu.edu/~tschwarz/coen252_04/Lectures/FAT.html
	//
	// Step 1: read the original "first cluster"
	uint32_t opaque = (uint32_t)read_halfword(buf, ptr + 26) ;
	// Step 2: add the higher 2 bytes.
	opaque += ((uint32_t)read_halfword(buf, ptr + 20)) << 16 ;
	de->opaque = (void*)opaque;
	// Print
	fat_debug_printf("FAT: read dir entry: %s, "
			 "size %d, cluster %d, ptr %d\n", 
			 de->name, de->byte_size, opaque, ptr);
      } // End for over the entries of the cluster
      
      // Get the next cluster
      if(is_root && (fs->fat_type != FAT32)) {
	cur_root_cluster_offset++;
	if(cur_root_cluster_offset < rpi_uidiv(fat->root_dir_sectors,
					       fat->sectors_per_cluster)) {
	  cur_cluster++;
	} else {
	  cur_cluster = 0x0ffffff8;
	}
      } else {
	cur_cluster = get_next_fat_entry(fat, cur_cluster);
      }
      fat_debug_printf("FAT: read dir: next cluster %x\n", cur_cluster);
    } while(cur_cluster < 0x0ffffff7);
  }
  return ret;
}

