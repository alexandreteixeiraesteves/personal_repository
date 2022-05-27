#include "myprog.h"


void Myprog__myread_step(int size, Myprog__myread_out*_out){
	float tmp[size];
	Sndlib__read_samples_out *o;
	Sndlib__read_samples_step(&size,tmp, o);
	_out->samples = tmp;
}

void Myprog__mywrite_step(int size, float *samples , Myprog__mywrite_out*_out){
	Sndlib__write_samples_out *o;
	Sndlib__write_samples_step(&size,samples, o) ; 
}
