
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

#ifndef FS_H
#define FS_H


#include <libc/stdint.h>
#include <libc/stddef.h> // For size_t


// End of file mark for text (ASCII) files.
#define EOF 0xff

// File opening modes
#define SEEK_SET        0x1000
#define SEEK_CUR        0x1001
#define SEEK_END        0x1002
#define SEEK_START      SEEK_SET

// Forward declaration, definition just after
// struct fs.
struct dirent ;
struct block_device ;
struct fs ;

// The FILE type.
typedef struct vfs_file {
  struct fs *fs;
  long pos;
  int mode;
  void *opaque;
  long len;
  int flags;
  int (*fflush_cb)(struct vfs_file *f);
} FILE ;

// File system description (with access functions).
struct fs {
  struct block_device *parent;
  const char *fs_name;
  uint32_t flags;
  size_t block_size;

  // Access functions specific to this filesystem.
  // Supports multiple filesystems.
  FILE *(*fopen)(struct fs *, struct dirent *, const char *mode);
  size_t (*fread)(struct fs *, void *ptr, size_t byte_size, FILE *stream);
  size_t (*fwrite)(struct fs *, void *ptr, size_t byte_size, FILE *stream);
  int (*fclose)(struct fs *, FILE *fp);
  long (*fsize)(FILE *fp);
  int (*fseek)(FILE *stream, long offset, int whence);
  long (*ftell)(FILE *fp);
  int (*fflush)(FILE *fp);
  
  struct dirent *(*read_directory)(struct fs *, char **name);
};

// Directory entry.
struct dirent {
  struct dirent *next;
  char name[32] ; // I fact, I only need 13, but I prefer putting 
                  // in more
  uint32_t byte_size;
  uint8_t is_dir;
  void *opaque;
  struct fs *fs;
};

// Block device.
struct block_device {
  char *driver_name;
  char *device_name;
  uint8_t *device_id;
  size_t dev_id_len;
  
  int supports_multiple_block_read;
  int supports_multiple_block_write;
  
  int (*read)(struct block_device *dev, uint8_t *buf, 
	      size_t buf_size, uint32_t block_num);
  int (*write)(struct block_device *dev, uint8_t *buf, 
	       size_t buf_size, uint32_t block_num);
  size_t block_size;
  size_t num_blocks;
  
  struct fs *fs;
};

struct dir_info {
  struct dirent *first;
  struct dirent *next;
};

typedef struct dir_info DIR ;


// Provided by libfs.c
int register_fs(struct block_device *dev, 
		int part_id);
int fs_interpret_mode(const char *mode);

// Provided by libfs.c
size_t fs_fread(uint32_t (*get_next_bdev_block_num)(uint32_t f_block_idx, 
						    FILE *s, 
						    void *opaque, 
						    int add_blocks),
		struct fs *fs, void *ptr, size_t byte_size,
		FILE *stream, void *opaque);
size_t fs_fwrite(uint32_t (*get_next_bdev_block_num)(uint32_t f_block_idx, 
						     FILE *s, 
						     void *opaque, 
						     int add_blocks),
		 struct fs *fs, void *ptr, size_t byte_size,
		 FILE *stream, void *opaque);

// Provided by block.c
size_t block_read(struct block_device *dev, uint8_t *buf, 
		  size_t buf_size, uint32_t starting_block);
size_t block_write(struct block_device *dev, uint8_t *buf, 
		   size_t buf_size, uint32_t starting_block);

// Provided by mbr.c
int read_mbr(struct block_device *parent, 
	     struct block_device ***partitions, 
	     int *part_count) ;

//
#define VFS_MODE_R	1
#define VFS_MODE_W	2
#define VFS_MODE_RW	3
#define VFS_MODE_APPEND	4
#define VFS_MODE_CREATE	8

#define VFS_FLAGS_EOF	1
#define VFS_FLAGS_ERROR	2

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
long fsize(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
int fflush(FILE *stream);
void rewind(FILE *stream);

int vfs_register(struct fs *fs);
void vfs_list_devices(void);
char **vfs_get_device_list(void);
int vfs_set_default(char *dev_name);
char *vfs_get_default(void);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
FILE *fopen(const char *path, const char *mode);
int fclose(FILE *fp);
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

// Provided by emmc.c
int sd_card_init(struct block_device **dev);

// Provided by fat.c
int fat_init(struct block_device *, struct fs **);

// Provided by libfs.c
void libfs_init(void) ;

#endif

