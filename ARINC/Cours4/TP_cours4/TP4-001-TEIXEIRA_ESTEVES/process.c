#include "myprog.h"
#include "main_c/main.h"

void main() {
	Main__main_out _out;
	for(;;) {
		Main__main_step(&_out);
	}
}
