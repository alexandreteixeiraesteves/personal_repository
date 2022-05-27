CC= gcc
CFLAGS= -I `heptc -where`/c     # options de compilation pour les sources C

#pre_src= complex_c/complex.c #complex_io.c
src= complex_c/complex_types.c complex_c/complex.c complex_io_types.c complex_io.c complexes_c/complexes_types.c complexes_c/complexes.c

main= main.c

src_obj= complex_types.o complex.o complex_io_types.o complex_io.o complexes_types.o complexes.o

main_obj= $(main:.c=.o)

#-I'complex_c' -I'complexes_c'

all:
	heptc -I `heptc -where`/c complex.ept
	heptc -I `heptc -where`/c -target c complex.ept
	$(CC) -I$(PWD) $(CFLAGS) -c complex_c/complex_types.c complex_c/complex.c
	heptc -c complex_io.epi
	$(CC) -I$(PWD) $(CFLAGS) -c complex_io_types.c complex_io.c 
	heptc -I `heptc -where`/c complexes.ept
	heptc -I `heptc -where`/c -target c complexes.ept
	$(CC) -I$(PWD) $(CFLAGS) -I'complex_c' -c complexes_c/complexes_types.c complexes_c/complexes.c
	$(CC) -I$(PWD) $(CFLAGS) -I'complex_c' -c $(main)
	$(CC) -I$(PWD) $(CFLAGS) $(src_obj) $(main_obj) -o main

clean: 
	rm -rf $(src_obj) $(main_obj) complex_c complexes_c main complexes_types.o complexes.o *.log *.obc *.mls *.epci

