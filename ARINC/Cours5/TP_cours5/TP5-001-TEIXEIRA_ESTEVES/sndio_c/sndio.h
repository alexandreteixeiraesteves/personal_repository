#ifndef SNDIO_H
#define SNDIO_H

#include <constants_types.h>
#define Constants__sample_size 256
typedef struct {
  float samples[Constants__sample_size] ;
} Sndio__read_samples_out ;
void Sndio__read_samples_step(int sample_size,Sndio__read_samples_out*o) ;

typedef struct {} Sndio__write_samples_out ;
void Sndio__write_samples_step(int sample_size,float*samples,
			       Sndio__write_samples_out*o) ;

#endif

