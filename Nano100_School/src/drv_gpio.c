#include "NANO1xx.h"
#include "nano1xx_gpio.h"

#include "drv_gpio.h"
#include "sys_util.h"

int DRV_GPIO_Init() {
	GPIO_Init();

	GPIO_Open(GPIOA, GPIO_PMD_PMD12_OUTPUT, GPIO_PMD_PMD12_MASK);	// For SSD1306 D/C Control @Nano100
	GPIO_Open(GPIOA, GPIO_PMD_PMD13_OUTPUT, GPIO_PMD_PMD13_MASK);	// For SPI-0 Power Control @Nano100

	GPIO_Open(GPIOB, GPIO_PMD_PMD9_OUTPUT, GPIO_PMD_PMD9_MASK);
	GPIO_Open(GPIOB, GPIO_PMD_PMD10_OUTPUT, GPIO_PMD_PMD10_MASK);
	GPIO_Open(GPIOB, GPIO_PMD_PMD11_OUTPUT, GPIO_PMD_PMD11_MASK);
	GPIO_Open(GPIOB, GPIO_PMD_PMD12_OUTPUT, GPIO_PMD_PMD12_MASK);

	GPIO_Open(GPIOC, GPIO_PMD_PMD6_OUTPUT, GPIO_PMD_PMD6_MASK);
	GPIO_Open(GPIOC, GPIO_PMD_PMD7_OUTPUT, GPIO_PMD_PMD7_MASK);		// For UART0 Power Control @Nano100

	GPIO_Open(GPIOE, GPIO_PMD_PMD5_OUTPUT, GPIO_PMD_PMD5_MASK);

	// For Nano130 POC Board
	//GPIO_Open(GPIOE, GPIO_PMD_PMD6_OUTPUT, GPIO_PMD_PMD6_MASK);

	// For Nano100 POC Board
	//GPIO_ClrBit(GPIOC,  7); // disable UART0 Power
	//GPIO_ClrBit(GPIOA, 13); // disable SPI-0 Power
	GPIO_SetBit(GPIOC,  7); //  enable UART0 Power
	GPIO_SetBit(GPIOA, 13); //  enable SPI-0 Power

	return 0;
}

int DRV_GPIO_Start() {

	return 0;
}

int DRV_GPIO_Set() {

	return 0;
}

int DRV_GPIO_Get() {

	return 0;
}

int DRV_GPIO_Stop() {

	return 0;
}
