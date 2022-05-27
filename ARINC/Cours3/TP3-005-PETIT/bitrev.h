#ifndef BITREV_H
#define BITREV_H
#include "bitrev_types.h"
#include "complex_c/complex.h"

typedef struct { struct Complex__complex o[16]; } Bitrev__bitrev16_out ;
typedef struct { struct Complex__complex o[8]; } Bitrev__bitrev8_out ;

void Bitrev__bitrev16_step(Complex__complex*i,Bitrev__bitrev16_out *o);
void Bitrev__bitrev8_step(Complex__complex*i,Bitrev__bitrev8_out *o);

#endif
