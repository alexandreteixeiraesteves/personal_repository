CC= gcc
CFLAGS= -I `heptc -where`/c     # options de compilation pour les sources C

main= main.c

main_obj= $(main:.c=.o)

all:
	heptc -I `heptc -where`/c complex.ept
	heptc -I `heptc -where`/c -target c complex.ept
	$(CC) -I$(PWD) $(CFLAGS) -c complex_c/complex_types.c complex_c/complex.c
	heptc -c complex_io.epi
	$(CC) -I$(PWD) $(CFLAGS) -c complex_io_types.c complex_io.c             
	heptc -c complex_vec_io.epi
	$(CC) -I$(PWD) $(CFLAGS) -c complex_vec_io_types.c complex_vec_io.c 
	heptc -I `heptc -where`/c complex_vec.ept
	heptc -I `heptc -where`/c -c complex_vec.ept #génération d'un fichier .epo pour les fonction template  
	heptc -I `heptc -where`/c complex_vectors.ept
	heptc -I `heptc -where`/c -target c complex_vectors.ept
	$(CC) -I$(PWD) $(CFLAGS) -I'complex_c' -I'complex_vec_c' -c complex_vec_c/complex_vec_types.c complex_vec_c/complex_vec.c complex_vectors_c/complex_vectors_types.c complex_vectors_c/complex_vectors.c
	heptc -c sndlib.epi
	$(CC) -I$(PWD) $(CFLAGS) -c sndlib.c
	heptc -c myprog.epi
	$(CC) -I$(PWD) $(CFLAGS) -c myprog.c
	heptc -I `heptc -where`/c main.ept
	heptc -I `heptc -where`/c -target c main.ept
	$(CC) -I$(PWD) $(CFLAGS) -I'complex_c' -c main_c/main_types.c main_c/main.c 
	$(CC) -I$(PWD) $(CFLAGS) -I'complex_c' -c process.c
	$(CC) -I$(PWD)  sndlib.o myprog.o main.o process.o -o main

run:
	rec -t raw -r 44100 -e signed -b 16 -c 2 - | ./main | play -t raw -r 44100 -e signed -b 16 -c 2 -

clean: 
	rm -rf main *.epo *.epci *.o *.log *.mls *.obc complex_c complex_vec_c complex_vectors_c main_c 

