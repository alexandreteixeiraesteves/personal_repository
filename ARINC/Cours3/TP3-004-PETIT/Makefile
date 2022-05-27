CC= gcc
CFLAGS= -std=c99 -I'build' # options de compilation pour les sources C
LDFLAGS = -lm -lc -lgcc
#pre_src= complex_c/complex.c #complex_io.c
src= complex_c/complex_types.c complex_c/complex.c complex_io_types.c complex_io.c complexes_c/complexes_types.c complexes_c/complexes.c

main= main.c

src_obj= complex_types.o complex.o complex_io_types.o complex_io.o complexes_types.o complexes.o twiddle_init.o twiddle.o 

main_obj= $(main:.c=.o)

#-I'complex_c' -I'complexes_c'

all:
	./compile.sh

clean: 
	rm -rf $(src_obj) $(main_obj) complex_c complexes_c bitrev_c complex_io_c complex_vec_io_c fft_c fft_test_c twiddle_c main *o *.log *.obc *.mls *.epci build built

