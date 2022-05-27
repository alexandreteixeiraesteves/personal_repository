#include "first_c/first.h"

void main() {
	First__main_out o;
	First__main_mem s;
	First__main_reset(&s);
	for(;;) {
		First__main_step(&o, &s);
	}
}
