#include <math.h>
#include <stdio.h>
#include <string.h>

#include "float.h"

void Float__float_asin_step (float i, Float__float_asin_out*o) {
  o->o = asinf(i) ;
}

void Float__float_acos_step (float i,      Float__float_acos_out*o) {
  o->o = acosf(i) ;
}
void Float__float_atan2_step(float im,float re,Float__float_atan2_out*o) {
  o->ang = atan2(im,re) ;
}
void Float__float_round_step(float i,      Float__float_round_out*o) {
  o->o = roundf(i) ;
}
void Float__float_sqrt_step (float i,      Float__float_sqrt_out*o) {
  o->o = sqrtf(i) ;
}
void Float__float_sin_step  (float i,      Float__float_sin_out*o) {
  o->o = sinf(i) ;
}
void Float__float_cos_step  (float i,      Float__float_cos_out*o) {
  o->o = cosf(i) ;
}
void Float__int2float_step  (int i,      Float__int2float_out*o) {
  o->o = (float)i ;
}
void Float__float2int_step (float i,Float__float2int_out*o) {
  o->o = (int)i ;
}
void Float__float_pow_step  (float i1,float i2,Float__float_pow_out*o) {
  o->o = pow(i1,i2) ;
}
void Float__int_is_odd_step(int i,Float__int_is_odd_out*o) {
  o->o = i&1 ;
}

