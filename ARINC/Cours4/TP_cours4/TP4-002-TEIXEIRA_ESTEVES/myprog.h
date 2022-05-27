#ifndef MYPROG_H
#define MYPROG_H

#include <stdio.h>
#include <stdlib.h>

#include "sndlib.h"

typedef struct { float *samples; } Myprog__myread_out ;
typedef struct { } Myprog__mywrite_out ;

void Myprog__myread_step(int size, Myprog__myread_out*_out) ;
void Myprog__mywrite_step(int size, float *samples ,Myprog__mywrite_out*_out) ;

#endif // MYPROG_H
