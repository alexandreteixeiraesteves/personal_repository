#ifndef SNDLIB_H
#define SNDLIB_H

typedef struct { } Sndlib__read_samples_out ;
typedef struct { } Sndlib__write_samples_out ;

void Sndlib__read_samples_step(int*sample_size,float*samples, Sndlib__read_samples_out *out) ;
void Sndlib__write_samples_step(int*sample_size,float*samples, Sndlib__write_samples_out *out) ;

#endif
