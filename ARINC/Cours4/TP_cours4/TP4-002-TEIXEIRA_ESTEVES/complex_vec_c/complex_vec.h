/* --- Generated the 6/5/2022 at 16:4 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c complex_vectors.ept --- */

#ifndef COMPLEX_VEC_H
#define COMPLEX_VEC_H

#include "complex_vec_types.h"
#include "complex.h"
typedef struct Complex_vec__vect_add_params_3__out {
  Complex__complex c[3];
} Complex_vec__vect_add_params_3__out;

void Complex_vec__vect_add_params_3__step(Complex__complex v2[3],
                                          Complex__complex v[3],
                                          Complex_vec__vect_add_params_3__out* _out);

typedef struct Complex_vec__sum_of_all_params_3__out {
  Complex__complex n_2;
} Complex_vec__sum_of_all_params_3__out;

void Complex_vec__sum_of_all_params_3__step(Complex__complex n_1[3],
                                            Complex_vec__sum_of_all_params_3__out* _out);

#endif // COMPLEX_VEC_H
