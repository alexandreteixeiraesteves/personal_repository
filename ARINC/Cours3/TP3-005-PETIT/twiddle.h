#ifndef TWIDDLE_V_H
#define TWIDDLE_V_H
#include "complex_c/complex.h"
#include "complex_io_types.h"
#include "twiddle_init.h"
typedef struct { struct Complex__complex o; } Twiddle__twiddle_out ;


void Twiddle__twiddle_step(int n, int k, Twiddle__twiddle_out* _out) ;

Complex__complex Twiddle__twiddle[1024];
#endif
