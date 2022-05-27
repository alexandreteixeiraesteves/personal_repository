CC= gcc 
CFLAGS= -I `heptc -where`/c  	# options de compilation pour les sources C

src= first_c/first.c

main= main.c

src_obj= $(src:.c=.o)

main_obj= $(main:.c=.o)

all: compile $(src_obj) $(main_obj)
	$(CC) $(src_obj) $(main_obj) -o main

check:
	heptc $(CFLAGS) first.ept

compile: check
	heptc $(CFLAGS) -target c first.ept
	
clean:
	rm $(src_obj) $(main_obj) main
	rm -rf *.epci *.log *.mls *.obc first_c


