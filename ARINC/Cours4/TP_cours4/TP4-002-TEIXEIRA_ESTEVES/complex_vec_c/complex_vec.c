/* --- Generated the 6/5/2022 at 16:4 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c complex_vectors.ept --- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex_vec.h"

void Complex_vec__vect_add_params_3__step(Complex__complex v2[3],
                                          Complex__complex v[3],
                                          Complex_vec__vect_add_params_3__out* _out) {
  Complex__complex_add_out Complex__complex_add_out_st;
  {
    int i;
    for (i = 0; i < 3; ++i) {
      Complex__complex_add_step(v2[i], v[i], &Complex__complex_add_out_st);
      _out->c[i] = Complex__complex_add_out_st.o;
    }
  };
}

void Complex_vec__sum_of_all_params_3__step(Complex__complex n_1[3],
                                            Complex_vec__sum_of_all_params_3__out* _out) {
  Complex__complex_add_out Complex__complex_add_out_st;
  
  Complex__complex n_3;
  n_3.im = 0.000000;
  n_3.re = 0.000000;
  _out->n_2 = n_3;
  {
    int i;
    for (i = 0; i < 3; ++i) {
      Complex__complex_add_step(n_1[i], _out->n_2,
                                &Complex__complex_add_out_st);
      _out->n_2 = Complex__complex_add_out_st.o;
    }
  };;
}

