#include <math.h>
#include <complex.h>
#include "twiddle_init.h"
#include <stdio.h>
float Complex__pi = 3.14159265358979;
void init_twiddle1024(Complex__complex* twiddle) {
  int i,span ;
  // Init the twiddles
  
  for(span=1;span<=512;span<<=1) {
    float primitive_root = -Complex__pi/span ;
    for(i=0;i<span;i++) {
      Complex__complex t =
	{
	  .re = cosf(primitive_root*i) ,
	  .im = sinf(primitive_root*i)
	} ;
      twiddle[span+i] = t ;
    }
  }
}
