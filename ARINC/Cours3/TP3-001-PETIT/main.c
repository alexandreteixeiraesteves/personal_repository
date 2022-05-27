#include <stdio.h>
#include "gnc_c/gnc.h"
#include "gnc_c/gnc_types.h"
#include <unistd.h>
int main() {
	Gnc__main_out o;
	Gnc__main_mem s;
	Gnc__main_reset(&s);
	for (;;) {
		usleep(1000000);
		Gnc__main_step(&o, &s);
	}
	return 0;
}
