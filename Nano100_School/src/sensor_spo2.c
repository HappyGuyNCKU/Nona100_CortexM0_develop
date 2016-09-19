#include "NANO1xx.h"
#include "nano1xx_gpio.h"

#include "sensor_spo2.h"

int SPO2_Init() {

	return 0;
}

int SPO2_Start() {
	GPIOC->DOUT |= (1 << 6);

	return 0;
}

int SPO2_Set() {

	return 0;
}

int SPO2_Get() {

	return 0;
}

int SPO2_Stop() {
	GPIOC->DOUT &= ~(1 << 6);

	return 0;
}
