#include "milkwm.h"

int main() {
	int st;

	wm_config_init();
	wm_config_read();

	if((st = init_x()) != 0) return st;

	loop_wm();
}
