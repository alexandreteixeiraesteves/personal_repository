#include "myprog.h"
#include "main_c/main.h"

void main() {
	Main__main_mem s;
	Main__main_out _out;
	Main__main_reset(&s);
	for(;;) {
		Main__main_step(&_out,&s);
	}
}
