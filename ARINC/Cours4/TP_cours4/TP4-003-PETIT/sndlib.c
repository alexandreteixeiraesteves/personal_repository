#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "sndlib.h"


// Structure of a sample for machines with 2 channels,
// signed 16-bit audio capture.
struct sample {
  int16_t left ;
  int16_t right ;
} ;

// The input/output buffer used for exchanging data
#define MAXBUFSIZE 2048
static struct sample buf[MAXBUFSIZE] ;

// Samples communicated to/from the application are single-channel and
// of float type, to facilitate integration.
void Sndlib__read_samples_step(int*sample_size,float*samples, Sndlib__read_samples_out *out) {
  // Check size fit 
  assert(*sample_size<=MAXBUFSIZE) ;
  // Reading the data from standard input
  int r ;
  int r_acc = sizeof(struct sample)*(*sample_size) ;
  char* tmp_buf = (char*)buf ;
  for(;r_acc;r_acc-=r,tmp_buf+=r) {
    r = read(0,tmp_buf,r_acc) ;
    if(r<0) {
      perror("read_samples error:") ;
      exit(0) ;
    } 
  }
  // Convert to SAMPLE_TYPE, and extract only one channel.
  for(r=0;r<*sample_size;r++) {
    samples[r] = (float)(buf[r].left) ;
  }
}

void Sndlib__write_samples_step(int*sample_size,float*samples, Sndlib__write_samples_out *out) {
  // Check size fit 
  assert(*sample_size<=MAXBUFSIZE) ;
  // Convert to struct sample, and duplicate the input data
  // on the two channels
  int w ;
  for(w=0;w<*sample_size;w++) {
    buf[w].left = (int32_t) samples[w] ;
    buf[w].right = buf[w].left ;
  }
  // Write the data
  char* tmp_buf = (char*)buf ;
  int w_acc = sizeof(struct sample)*(*sample_size) ;
  for(;w_acc;w_acc-=w,tmp_buf+=w) {
    w = write(1,tmp_buf,w_acc) ;
    if(w<0) {
      perror("write_samples error:") ;
      exit(0) ;
    }
  }
  fflush(stdout) ;
}

// Fonction "main" permettant le test.
// Il faut la commenter pour la version "librairie".
/*
void main(void) {
  int size = 128 ;
  float abc[size] ;
  for(;;) {
    Sndlib__read_samples_step(&size,abc) ;
    Sndlib__write_samples_step(&size,abc) ;
  }
}
*/
