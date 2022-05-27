
#include <stdio.h>
#include "externc.h"

void Externc__gnc_print_step(int i, int idx, int y, Externc__gnc_print_out*_out) {
	printf("GNC(y = %d, idx = %d)=%d\n",i, idx, y) ;
}

void Externc__fast_print_step(int i, int idx, int y, Externc__fast_print_out*_out) {
	printf("FAST(x = %d, idx = %d)=%d\n",i, idx, y) ;
}
void Externc__thermal_print_step(int idx, Externc__thermal_print_out*_out){
	printf("THERMAL(%d)\n",idx);
}
void Externc__act_step(int addr, Externc__act_out*_out) {
	return;
}
