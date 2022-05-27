
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
#include <libc/string.h>

/* Helper function that zeroes a memory area. */
/* It can be called in the kernel as soon as  */
/* a valid stack has been set.                */
void bzero(void *p, size_t n) {
  char *s = (char*)p ;
  // For the case where s is not word-aligned,
  // use non-aligned accesses. 
  while(((sizeof(uint32_t)-1) & (uint32_t)s)&&
	(n>0)) {
    *s++ = 0 ;
    n-- ;
  }
  // Now, zero by chunks of 4 words.
  // This part should be optimized 
  // using assembly code. 
  char* t = (char*)(s+(n&(~(4*sizeof(uint32_t)-1)))) ;  
  n -= n&(~(4*sizeof(uint32_t)-1)) ; // The remainder
  while(s!=t) {
    size_t *su = (size_t*)s ;
    su[0] = 0 ;
    su[1] = 0 ;
    su[2] = 0 ;
    su[3] = 0 ;
    s += 4*sizeof(size_t) ;
  }
  // We still have to zero word by word.
  t = (char*)(s+(n&(~(sizeof(uint32_t)-1)))) ;
  n -= n&(~(sizeof(uint32_t)-1)) ;
  while(s!=t) {
    ((uint32_t*)s)[0] = 0 ;
    s += sizeof(uint32_t) ;
  }
  t = s + n ;
  while(s!=t) {
    *s++ = 0 ;
  }
}

void*  memcpy(void *restrict s1, const void *restrict s2, size_t n) {
  uint32_t i ;
  for(i=0;i<n;i++){
    ((char*)s1)[i] = ((char*)s2)[i] ;
  }
  return s1 ;
} 

void*  memset(void *b, int c, size_t len) {
  register unsigned char c1 = (unsigned char)c ;
  uint32_t i ;
  for(i=0;i<len;i++){
    ((unsigned char*)b)[i] = c1 ;
  }
  return b ;  
}

size_t strlen(const char *s) {
  uint32_t i ;
  for(i=0;s[i];i++) ;
  return i ;
}

char*  strcpy(char *dst, const char *src) {
  char* result = dst ;
  for(;;) {
    *dst = *src ;
    if(!(*src)) return result ; 
    dst++ ;
    src++ ;
  }
}

char*  strncpy(char *restrict s1, const char *restrict s2, 
	       size_t n) {
  uint32_t i ;
  // Copy at most n chars from s2 to s1. Stop at either n
  // or at the end of s2.
  for(i=0;(i<n)&&(s2[i]);i++) {
    s1[i] = s2[i] ;
  }
  // If s2 was shorter than s1, then fill the remainder
  // of s1 with \0.
  for(;i<n;i++) {
    s1[i] = 0 ;
  }
  return s1 ;
}

size_t strlcpy(char *restrict s1, const char *restrict s2, 
	       size_t n) {
  uint32_t i ;
  // Copy at most n chars from s2 to s1. Stop at either n
  // or at the end of s2.
  for(i=0;(i<(n-1))&&(s2[i]);i++) {
    s1[i] = s2[i] ;
  }
  // Null-terminate
  s1[i] = 0 ;
  return i ;
}

int    strcmp(const char *s1, const char *s2) {
  for(;;) {
    register unsigned char v1 = *s1 ;
    register unsigned char v2 = *s2 ;
    if(v1 > v2) return 1 ;
    if(v2 > v1) return -1 ;
    if(!v1) return 0 ;
    s1++ ; s2++ ;
  }
}

int    strncmp(const char *s1, const char *s2, size_t n) {
  uint32_t i ;
  for(i=0;i<n;i++) {
    register unsigned char v1 = *s1 ;
    register unsigned char v2 = *s2 ;
    if(v1 > v2) return 1 ;
    if(v2 > v1) return -1 ;
    if(!v1) return 0 ;
    s1++ ; s2++ ;
  }
  return 0 ; // by default, it's equality.
}
