#include "complexes_c/complexes.h"

void main() {
	Complexes__complexes_out o;
	for(;;) {
		Complexes__complexes_step(&o);
	}
}
