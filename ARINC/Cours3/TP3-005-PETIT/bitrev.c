// This function is adapted from
//   http://www.katjaas.nl/bitreversal/bitreversal.html

// Change this type with the type of your choice
// (in this TP, Complex__complex, corresponding to
// the complex type defined in Heptagon,
// in file complex.ept).
#include "bitrev.h"
#include <string.h>
#define datatype Complex__complex
static inline void swap(unsigned int forward,
			unsigned int rev,
			datatype *data) {
  datatype tmp;  
  tmp = data[forward];               
  data[forward] = data[rev];
  data[rev] = tmp;
}
// Main function
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
// Instantiation for size 16, working with Heptagon's
// calling conventions.
void Fftc__bitrev16_step(Complex__complex*i, Complex__complex *o) {
  memcpy(o,i,16*sizeof(Complex__complex)) ;
  bitrev(o,4) ;
}
void Fftc__bitrev8_step(Complex__complex*i,Complex__complex *o) {
  memcpy(o,i,8*sizeof(Complex__complex)) ;
  bitrev(o,3) ;
}
void Bitrev__bitrev16_step(Complex__complex*i,Bitrev__bitrev16_out *_out) {
	Fftc__bitrev16_step(i, _out->o);
}
void Bitrev__bitrev8_step(Complex__complex*i,Bitrev__bitrev8_out *_out) {
	Fftc__bitrev8_step(i, _out->o);
}

