#include <stdio.h>
#include "extern.h"
#include "extern_types.h"

int main() {
	Extern__f_out _outf;
	Extern__g_out _outg;
	_outg.o = 0;
	int z = 0;
	for (;;) {
		if (_outg.o == 0) {
			z = 123;
		}
		else {
			z = _outg.o;
		}
		Extern__f_step(z, &_outf);
		
		Extern__g_step(_outf.o, &_outg);
		sleep(1);	
	}	  
	return 0  ;
}
