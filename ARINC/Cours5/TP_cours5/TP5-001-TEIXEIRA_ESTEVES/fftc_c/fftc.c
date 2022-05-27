#include <math.h>
#include <stdio.h>
#include <string.h>

#include <fftc.h>

float Fftc__hann[FFT_SIZE] ;
Complex__complex Fftc__twiddle[FFT_SIZE] ;


void Fftc__get_twiddle_step(int k, int n, Fftc__get_twiddle_out*o) {
  o->o = Fftc__twiddle[n/2+k] ;
}

void Fftc__get_hann1024_step(int k, Fftc__get_hann1024_out*o) {
  o->o = Fftc__hann[k] ;
}


void init_fft(void) {
  int i,span ;
  // Init the Hann window
  for(i=0;i<FFT_SIZE;i++) {
    Fftc__hann[i] = (1-cosf(Complex__pi*2*i/FFT_SIZE))/2 ;
  }
  // Init the twiddles
  for(span=1;span<=FFT_SIZE/2;span<<=1) {
    float primitive_root = -Complex__pi/span ;
    for(i=0;i<span;i++) {
      Complex__complex twiddle =
	{
	  .re = cosf(primitive_root*i) ,
	  .im = sinf(primitive_root*i)
	} ;
      Fftc__twiddle[span+i] = twiddle ;
    }
  }
}

#define datatype Complex__complex
static inline void swap(unsigned int forward,
			unsigned int rev,
			datatype *data) {
  datatype tmp;  
  tmp = data[forward];               
  data[forward] = data[rev];
  data[rev] = tmp;
}
void bitrev(datatype *data, unsigned int logN)
{
  // Initialization of frequently used constants
  const unsigned int N = 1<<logN;
  const unsigned int halfn = N>>1;   
  const unsigned int quartn = N>>2;
  const unsigned int nmin1 = N-1;

  // Variables
  unsigned int i, forward, rev, zeros;
  unsigned int nodd, noddrev;

  // Variable initialization
  forward = halfn;
  rev = 1;

  // start of bitreversed permutation loop, N/4 iterations
  for(i=quartn; i; i--) {
    // Gray code generator for even values:      
    nodd = ~i;                                  // counting ones is easier
    for(zeros=0; nodd&1; zeros++) nodd >>= 1;   // find trailing zeros in i
    forward ^= 2 << zeros;                      // toggle one bit of forward
    rev ^= quartn >> zeros;                     // toggle one bit of rev
    
    // swap even and ~even conditionally
    if(forward<rev) {
      swap(forward, rev, data);
      nodd = nmin1 ^ forward;                   // compute the bitwise negations
      noddrev = nmin1 ^ rev;       
      swap(nodd, noddrev, data);                // swap bitwise-negated pairs
    }
      
    nodd = forward ^ 1;                         // compute the odd values from the even
    noddrev = rev ^ halfn;
    swap(nodd, noddrev, data);                  // swap odd unconditionally
  }   
}

void Fftc__bitrev1024_step(Complex__complex*i,Fftc__bitrev1024_out*o) {
  memcpy(o->o,i,1024*sizeof(Complex__complex)) ;
  bitrev(o->o,10) ;
}

void Fftc__bitrev8_step(Complex__complex*i,Fftc__bitrev8_out*o) {
  memcpy(o->o,i,8*sizeof(Complex__complex)) ;
  bitrev(o->o,3) ;
}

void Fftc__print_complex_vector_step(Complex__complex*i,
				     Fftc__print_complex_vector_out*o) {
  printf("[") ;
  for(int j=0;j<8;j++)
    printf("(%f,%f);",i[j].re, i[j].im) ;
  printf("]\n") ;
}
