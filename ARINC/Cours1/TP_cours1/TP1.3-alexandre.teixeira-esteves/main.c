#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "first_c/first.h"

void main() {
	First__counter_out o ; /* Allocation des sorties */
	First__counter_mem s ; /* Allocation d l'état */
	First__counter_reset(&s) ; /* Initialisation de l'état */
	for(;;) { /* Boucle infinie */
		usleep(1000000); //wait_trigger() ; 
		/* A vous d'en choisir un (timer, input, etc.) */
		First__counter_step(&o,&s) ;
		printf(" Result: cnt=%d\n",o.cnt) ;
	}
}
