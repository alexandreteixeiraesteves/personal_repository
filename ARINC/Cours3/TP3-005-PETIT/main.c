#include <stdio.h>
#include "twiddle.h"
#include "fft_test_c/fft_test.h"
int main() {
	Fft_test__fft8_out* _out;
	//Complex_io__print_complex_out* _ot;
	init_twiddle1024(Twiddle__twiddle);	
	/*for(int i= 0; i < 1024; i++) {
		Complex_io__print_complex_step(Twiddle__c[i], _ot);
	}
	printf("main\n");
	fflush(stdout);*/
	
	Fft_test__fft8_step(_out);
	
	return 0;
}
