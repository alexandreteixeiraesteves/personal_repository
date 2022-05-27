#include <stdio.h>
#include "complex_io.h"
void Complex_io__read_complex_step(Complex_io__read_complex_out*_out){
	printf("Re:"); scanf("%e",&(_out->o.re)) ; /* Lecture des entrées */
	printf("Im:"); scanf("%e",&(_out->o.im)) ; /* Lecture des entrées */
}

void Complex_io__print_complex_step(struct Complex__complex _in, Complex_io__print_complex_out*_out){
	printf("Result => %f + i%f\n",_in.re, _in.im) ;
}
