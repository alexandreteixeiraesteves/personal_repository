#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "first_c/first.h"

void main() {
	int r;		/* Allocation des entrées */
	First__rcounter_out o ; /* Allocation des sorties */
	First__rcounter_mem s ; /* Allocation d l'état */
	First__rcounter_reset(&s) ; /* Initialisation de l'état */
	for(;;) { /* Boucle infinie */ 
		/* A vous d'en choisir un (timer, input, etc.) */
		printf("Reset:"); scanf("%d",&r) ;
		First__rcounter_step(r,&o,&s) ;
		printf(" Result: cnt=%d\n",o.cnt) ;
	}
}
