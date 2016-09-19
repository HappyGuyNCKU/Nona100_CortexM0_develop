#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sys_shell.h"

int shell_open() {
	sys_sensorhub_init();
	sys_sensors_init();

	return 0;
}

int shell_process() {

	return 0;
}

int shell_close() {

	return 0;
}
