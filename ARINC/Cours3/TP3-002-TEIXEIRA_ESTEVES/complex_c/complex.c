/* --- Generated the 15/4/2022 at 19:44 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c complex.ept --- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex.h"

void Complex__complex_add_step(Complex__complex i1, Complex__complex i2,
                               Complex__complex_add_out* _out) {
  
  float v_1;
  float v;
  float a;
  float b;
  float c;
  float d;
  d = i2.im;
  c = i2.re;
  b = i1.im;
  a = i1.re;
  v_1 = (b+d);
  v = (a+c);
  _out->o.re = v;
  _out->o.im = v_1;;
}

void Complex__complex_sub_step(Complex__complex i1, Complex__complex i2,
                               Complex__complex_sub_out* _out) {
  
  float v_2;
  float v;
  float a;
  float b;
  float c;
  float d;
  d = i2.im;
  c = i2.re;
  b = i1.im;
  a = i1.re;
  v_2 = (b-d);
  v = (a-c);
  _out->o.re = v;
  _out->o.im = v_2;;
}

void Complex__complex_mul_step(Complex__complex i1, Complex__complex i2,
                               Complex__complex_mul_out* _out) {
  
  float v_7;
  float v_6;
  float v_5;
  float v_4;
  float v_3;
  float v;
  float a;
  float b;
  float c;
  float d;
  d = i2.im;
  c = i2.re;
  b = i1.im;
  a = i1.re;
  v_6 = (b*c);
  v_5 = (a*d);
  v_7 = (v_5+v_6);
  v_3 = (b*d);
  v = (a*c);
  v_4 = (v-v_3);
  _out->o.re = v_4;
  _out->o.im = v_7;;
}

