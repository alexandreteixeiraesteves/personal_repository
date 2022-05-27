#ifndef COMPLEX_VEC_IO
#define COMPLEX_VEC_IO

#include <stdio.h>
#include <stdlib.h>

#include "complex_c/complex.h"

typedef struct { struct Complex__complex o[8]; } Complex_vec_io__read_complex_vector_out ;
typedef struct { } Complex_vec_io__print_complex_vector_out ;

void Complex_vec_io__read_complex_vector_step(Complex_vec_io__read_complex_vector_out*_out) ;
void Complex_vec_io__print_complex_vector_step(struct Complex__complex c[3],Complex_vec_io__print_complex_vector_out*_out) ;

#endif // COMPLEX_VEC_IO_H
