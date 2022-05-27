#include "complex_io.h"

void Complex_io__read_complex_step(Complex_io__read_complex_out*_out){
	scanf("%f",&(_out->o.re)) ;
	scanf("%f",&(_out->o.im)) ;
}

void Complex_io__print_complex_step(struct Complex__complex c,Complex_io__print_complex_out*_out){
	printf(" %f + i %f \n", c.re, c.im);
}
