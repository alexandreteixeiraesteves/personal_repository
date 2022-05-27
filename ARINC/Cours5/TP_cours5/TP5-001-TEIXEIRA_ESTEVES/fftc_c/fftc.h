#ifndef FFTC_H
#define FFTC_H

#include <constants_types.h>
#include <complex_types.h>

#define FFT_SIZE 1024
#define FFT_SIZE_LOG 10
#define SAMPLE_SIZE 256

/* Constant vectors, initialized in C */
extern float Fftc__hann[FFT_SIZE] ;
extern Complex__complex Fftc__twiddle[FFT_SIZE] ;
void init_fft(void);

typedef struct {
  Complex__complex o ;
} Fftc__get_twiddle_out ;
void Fftc__get_twiddle_step(int k, int n, Fftc__get_twiddle_out*o) ;

typedef struct {
  float o ;
} Fftc__get_hann1024_out ;
void Fftc__get_hann1024_step(int k, Fftc__get_hann1024_out*o) ;

typedef struct {
  Complex__complex o[1024] ;
} Fftc__bitrev1024_out ;
void Fftc__bitrev1024_step(Complex__complex*i,Fftc__bitrev1024_out*o) ;

typedef struct {
  Complex__complex o[8] ;
} Fftc__bitrev8_out ;
void Fftc__bitrev8_step(Complex__complex*i,Fftc__bitrev8_out*o) ;

typedef struct {} Fftc__print_complex_vector_out ;
void Fftc__print_complex_vector_step(Complex__complex*i,
				     Fftc__print_complex_vector_out*o) ;
#endif
