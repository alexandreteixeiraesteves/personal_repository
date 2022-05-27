#include "complex_vec_io.h"

void Complex_vec_io__read_complex_vector_step(Complex_vec_io__read_complex_vector_out*_out){
	for( size_t i = 0 ; i < 8 ; i++){
		struct Complex__complex tmp;
		scanf("%f",&tmp.re) ;
        	scanf("%f",&tmp.im) ;
		_out->o[i] = tmp;
	}
}

void Complex_vec_io__print_complex_vector_step(struct Complex__complex c[8], Complex_vec_io__print_complex_vector_out*_out){
	
	for( size_t i = 0 ; i < 8 ; i++){
		struct Complex__complex tmp = c[i];
		printf(" %f + i %f \n", tmp.re, tmp.im);
	}
	printf("\n");
}
