/* --- Generated the 6/5/2022 at 16:4 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c main.ept --- */

#ifndef MAIN_H
#define MAIN_H

#include "main_types.h"
#include "complex.h"
#include "myprog.h"
typedef struct Main__float_to_complex_out {
  Complex__complex c;
} Main__float_to_complex_out;

void Main__float_to_complex_step(float f, Main__float_to_complex_out* _out);

typedef struct Main__complex_to_float_out {
  float f;
} Main__complex_to_float_out;

void Main__complex_to_float_step(Complex__complex c,
                                 Main__complex_to_float_out* _out);

typedef struct Main__main_mem {
  float s1[256];
  float s2[256];
  float s3[256];
} Main__main_mem;

typedef struct Main__main_out {
} Main__main_out;

void Main__main_reset(Main__main_mem* self);

void Main__main_step(Main__main_out* _out, Main__main_mem* self);

#endif // MAIN_H
