#ifndef EXTERN_H
#define EXTERN_H
#include "extern_types.h"

typedef struct { int o; } Extern__f_out ;
typedef struct { int o; } Extern__g_out ;

void Extern__f_step(int i, Extern__f_out* _out);
void Extern__g_step(int i, Extern__g_out* _out);

#endif
