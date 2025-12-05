#include "milkwm.h"

/* we prefix them for reasons */

config_t wm_config;

void wm_config_init(void) {
	config_init(&wm_config);
}

static void read_config(const char* path) {
	if(!config_read_file(&wm_config, path)) {
		fprintf(stderr, "MilkWM config error: file %s at line %d: %s\n", path, config_error_line(&wm_config), config_error_text(&wm_config));
	}
}

void wm_config_read(void) {
	char* conf = MDEDirectoryConfigPath();
	char* dir  = MwDirectoryJoin(conf, "milkwm");
	char* path = MwDirectoryJoin(dir, "milkwmrc");

	free(dir);
	free(conf);

	config_clear(&wm_config);
	read_config(SYSCONFDIR "/milkwm/milkwmrc");
	read_config(path);

	free(path);
}
