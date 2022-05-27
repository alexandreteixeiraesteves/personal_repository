#include <stdio.h>

#include <fftc.h>

#define AUX_MACRO(x) x
#include AUX_MACRO(HEADER)
#include AUX_MACRO(THEADER)

#define CAT_AUX(x,y) x##y
#define CAT(x,y) CAT_AUX(x,y)

int main() {
  init_fft() ;
  CAT(MODNAME, __main_mem) mem ;
  CAT(MODNAME, __main_out) out ;
  CAT(MODNAME, __main_reset)(&mem) ;
  for(;;) {
    CAT(MODNAME, __main_step)(&out,&mem) ;
  }

  return 0  ;
}
