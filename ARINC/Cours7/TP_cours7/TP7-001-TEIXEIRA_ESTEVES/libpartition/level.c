#include <libpartition/svc-call.h>

uint32_t read_level() {
  uint32_t res ;
  svc_read_level(&res) ;
  return res ;
}
