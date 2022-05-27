CC= gcc
CFLAGS= -I `heptc -where`/c     # options de compilation pour les sources C

main= main.c

main_obj= $(main:.c=.o)

all:
	./compile.sh

run:
	rec -t raw -r 44100 -e signed -b 16 -c 2 - | ./main | play -t raw -r 44100 -e signed -b 16 -c 2 -

clean: 
	rm -rf main *.epo *.epci *.o *.log *.mls *.obc complex_c complex_vec_c complex_vectors_c main_c bitrev_c complexes_c complex_io_c complex_vec_io_c fft_c ifft_c build

