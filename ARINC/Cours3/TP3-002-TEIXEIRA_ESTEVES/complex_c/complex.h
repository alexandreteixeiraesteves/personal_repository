/* --- Generated the 15/4/2022 at 19:44 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c complex.ept --- */

#ifndef COMPLEX_H
#define COMPLEX_H

#include "complex_types.h"
typedef struct Complex__complex_add_out {
  Complex__complex o;
} Complex__complex_add_out;

void Complex__complex_add_step(Complex__complex i1, Complex__complex i2,
                               Complex__complex_add_out* _out);

typedef struct Complex__complex_sub_out {
  Complex__complex o;
} Complex__complex_sub_out;

void Complex__complex_sub_step(Complex__complex i1, Complex__complex i2,
                               Complex__complex_sub_out* _out);

typedef struct Complex__complex_mul_out {
  Complex__complex o;
} Complex__complex_mul_out;

void Complex__complex_mul_step(Complex__complex i1, Complex__complex i2,
                               Complex__complex_mul_out* _out);

#endif // COMPLEX_H
