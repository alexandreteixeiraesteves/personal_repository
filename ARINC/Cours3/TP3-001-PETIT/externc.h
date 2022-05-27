#ifndef EXTERNC_H
#define EXTERNC_H
typedef struct { } Externc__gnc_print_out ;
typedef struct { } Externc__fast_print_out ;
typedef struct { } Externc__thermal_print_out ;
typedef struct { } Externc__act_out ;

void Externc__gnc_print_step(int i, int idx, int y, Externc__gnc_print_out*_out) ;
void Externc__fast_print_step(int i, int idx, int y, Externc__fast_print_out*_out) ;
void Externc__thermal_print_step(int i, Externc__thermal_print_out*_out) ;
void Externc__act_step(int addr, Externc__act_out*_out) ;

#endif
