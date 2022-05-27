#include <stdio.h>
#include "extern.h"
void Extern__f_step(int i, Extern__f_out* _out){
	_out->o = i + 1;
	printf("f(%d) = %d\n", i, _out->o);
}

void Extern__g_step(int i, Extern__g_out* _out){
	_out->o = i + 5;
	printf("g(%d) = %d\n", i, _out->o);
}
