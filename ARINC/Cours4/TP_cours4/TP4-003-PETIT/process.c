#include "myprog.h"
#include "main_c/main.h"
#include "twiddle.h"
void main() {
	Main__main_mem s;
	Main__main_out _out;
	Main__main_reset(&s);
	init_twiddle1024(Twiddle__twiddle);	
	for(;;) {
		Main__main_step(&_out,&s);
	}
}
