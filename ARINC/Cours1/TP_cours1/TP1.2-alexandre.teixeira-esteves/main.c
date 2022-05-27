#include <stdio.h>
#include <stdlib.h>

#include "first_c/first.h"

void main() {
	int i ; /* Allocation des entrées */
	First__sum2_out o ; /* Allocation des sorties */
	First__sum2_mem s ; /* Allocation d l'état */
	First__sum2_reset(&s) ; /* Initialisation de l'état */
	for(;;) { /* Boucle infinie */
		printf("Inputs:"); scanf("%d",&i) ; /* Lecture des entrées */
		First__sum2_step(i,&o,&s) ; /* Calculs */
		printf("Result: o=%d\n",o.o) ; /* Ecriture des sorties */
	}
}
