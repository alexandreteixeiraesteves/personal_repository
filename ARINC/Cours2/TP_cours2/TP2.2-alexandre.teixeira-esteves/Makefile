CC= gcc
CFLAGS= -I `heptc -where`/c     # options de compilation pour les sources C

src= externc.c first_c/first.c

main= main.c

src_obj= externc.o first.o 

main_obj= $(main:.c=.o)

all:
	heptc -c externc.epi
	heptc -I `heptc -where`/c first.ept
	heptc -I `heptc -where`/c -target c first.ept
	$(CC) -I$(PWD) $(CFLAGS) -c $(src)
	$(CC) -I$(PWD) $(CFLAGS) -c $(main)
	$(CC) -I$(PWD) $(src_obj) $(main_obj) -o main

clean: 
	rm -rf $(src_obj) $(main_obj) first_c main *.log *.obc *.mls *.epci

