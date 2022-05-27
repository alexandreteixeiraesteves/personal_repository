#include <librpi/debug.h>          // For debug_print

void Externc__read_bool_sensor_step(int* addr, int* sensor_value) {
  if(*addr == 0x1000) {
    // FS sensor behavior
    static int cnt = 0 ;
    cnt++ ;
    // fail after 6 cycles
    if(cnt > 5) *sensor_value = 1 ;
    else *sensor_value = 0 ;
    debug_printf("read_sensor(FS) = %d.\n",*sensor_value) ;
  } else if (*addr == 0x2000) {
    // HS sensor behavior
    static int cnt = 0 ;
    cnt++ ;
    // be in (not HS) once every 3 cycles
    // starting in the first
    if(cnt%3) *sensor_value = 1 ;
    else *sensor_value = 0 ;
    debug_printf("read_sensor(HS) = %d.\n",*sensor_value) ;
  }
}

void Externc__g_step(int* out) {
  static int cnt = 0 ;
  cnt ++ ;
  *out = cnt ;
  debug_printf("G() = %d.\n",*out) ;
}

void Externc__f1_step(int* i, int* o) {
  *o = (*i)+1 ;
  debug_printf("F1(%d) = %d.\n",*i,*o) ;
}

void Externc__f2_step(int* i, int* o) {
  *o = (*i)+2 ;
  debug_printf("F2(%d) = %d.\n",*i,*o) ;
}

void Externc__act_step(int* i) {
  debug_printf("ACT(%d).\n",*i) ;
}
