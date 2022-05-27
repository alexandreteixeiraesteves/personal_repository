#include "externc.h"

void Externc__read_bool_step(int addr,Externc__read_bool_out*_out) {
	printf("read_bool(%d):",addr) ; 
	fflush(stdout) ;
	scanf("%d",&(_out->value)) ;
}

void Externc__f1_step(int i,Externc__f1_out*_out){
	_out->o = i + 5 ;
        printf("F1(%d)=%d\n",i,_out->o) ;
}


void Externc__f2_step(int i,Externc__f2_out*_out) {
	_out->o = i + 100 ;
	printf("F2(%d)=%d\n",i,_out->o) ;
}

void Externc__g_step(Externc__g_out*_out) {
	static int s = 300 ;
	s += 50 ;
	_out->o = s ;
	printf("G()=%d\n",_out->o) ;
}

void Externc__act_step(int addr, Externc__act_out*_out){
	printf("act()=%d\n", addr);
}
