/* --- Generated the 15/4/2022 at 19:44 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c complexes.ept --- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complexes.h"

void Complexes__complexes_step(Complexes__complexes_out* _out) {
  Complex__complex_add_out Complex__complex_add_out_st;
  Complex_io__read_complex_out Complex_io__read_complex_out_st;
  Complex_io__print_complex_out Complex_io__print_complex_out_st;
  
  Complex__complex c1;
  Complex__complex c2;
  Complex__complex sum;
  Complex_io__read_complex_step(&Complex_io__read_complex_out_st);
  c2 = Complex_io__read_complex_out_st.o;
  Complex_io__print_complex_step(c2, &Complex_io__print_complex_out_st);
  Complex_io__read_complex_step(&Complex_io__read_complex_out_st);
  c1 = Complex_io__read_complex_out_st.o;
  Complex__complex_add_step(c1, c2, &Complex__complex_add_out_st);
  sum = Complex__complex_add_out_st.o;
  Complex_io__print_complex_step(sum, &Complex_io__print_complex_out_st);
  Complex_io__print_complex_step(c1, &Complex_io__print_complex_out_st);;
}

