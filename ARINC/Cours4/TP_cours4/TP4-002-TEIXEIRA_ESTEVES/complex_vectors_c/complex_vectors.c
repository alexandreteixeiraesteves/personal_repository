/* --- Generated the 6/5/2022 at 16:4 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c complex_vectors.ept --- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex_vectors.h"

void Complex_vectors__vectors_step(Complex_vectors__vectors_out* _out) {
  Complex_vec_io__print_complex_vector_out Complex_vec_io__print_complex_vector_out_st;
  Complex_vec__sum_of_all_params_3__out Complex_vec__sum_of_all_params_3__out_st;
  Complex_vec_io__read_complex_vector_out Complex_vec_io__read_complex_vector_out_st;
  Complex_vec__vect_add_params_3__out Complex_vec__vect_add_params_3__out_st;
  Complex_io__print_complex_out Complex_io__print_complex_out_st;
  
  Complex__complex v1[3];
  Complex__complex v2[3];
  Complex__complex v[3];
  Complex__complex c;
  Complex_vec_io__read_complex_vector_step(&Complex_vec_io__read_complex_vector_out_st);
  {
    int _1;
    for (_1 = 0; _1 < 3; ++_1) {
      v2[_1] = Complex_vec_io__read_complex_vector_out_st.o[_1];
    }
  };
  Complex_vec_io__read_complex_vector_step(&Complex_vec_io__read_complex_vector_out_st);
  {
    int _2;
    for (_2 = 0; _2 < 3; ++_2) {
      v1[_2] = Complex_vec_io__read_complex_vector_out_st.o[_2];
    }
  };
  Complex_vec__vect_add_params_3__step(v1, v2,
                                       &Complex_vec__vect_add_params_3__out_st);
  {
    int _3;
    for (_3 = 0; _3 < 3; ++_3) {
      v[_3] = Complex_vec__vect_add_params_3__out_st.c[_3];
    }
  };
  Complex_vec_io__print_complex_vector_step(v,
                                            &Complex_vec_io__print_complex_vector_out_st);
  Complex_vec__sum_of_all_params_3__step(v,
                                         &Complex_vec__sum_of_all_params_3__out_st);
  c = Complex_vec__sum_of_all_params_3__out_st.n_2;
  Complex_io__print_complex_step(c, &Complex_io__print_complex_out_st);;
}

