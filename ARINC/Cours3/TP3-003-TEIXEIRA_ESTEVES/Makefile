CC= gcc
CFLAGS= -I `heptc -where`/c     # options de compilation pour les sources C

src= complex_c/complex_types.c complex_c/complex.c complex_io_types.c complex_io.c complex_vec_io_types.c complex_vec_io.c complex_vec_c/complex_vec_types.c complex_vec_c/complex_vec.c complex_vectors_c/complex_vectors_types.c complex_vectors_c/complex_vectors.c

main= main.c

src_obj= complex_types.o complex.o complex_io_types.o complex_io.o complex_vec_io_types.o complex_vec_io.o complex_vec_types.o complex_vec.o complex_vectors_types.o complex_vectors.o

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
	$(CC) -I$(PWD) $(CFLAGS) -I'complex_c' -I'complex_vec_c' -c $(main)
	$(CC) -I$(PWD) $(src_obj) $(main_obj) -o main

clean: 
	rm -rf $(src_obj) $(main_obj) complex_c complex_vec_c complex_vectors_c *.epo main *.o *.log *.obc *.mls *.epci

