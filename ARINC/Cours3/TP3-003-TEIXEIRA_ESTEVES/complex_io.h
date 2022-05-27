#ifndef COMPLEX_IO_H
#define COMPLEX_IO_H

#include <stdio.h>
#include <stdlib.h>

#include "complex_io_types.h"
#include "complex_c/complex.h"

typedef struct { struct Complex__complex o; } Complex_io__read_complex_out ;
typedef struct { } Complex_io__print_complex_out ;

void Complex_io__read_complex_step(Complex_io__read_complex_out*_out) ;
void Complex_io__print_complex_step(struct Complex__complex c,Complex_io__print_complex_out*_out) ;

#endif // COMPLEX_IO_H
