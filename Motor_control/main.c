#include "NANO1xx.h"
#include "nano1xx_gpio.h"

int main(void){

	GPIO_Open(GPIOB, GPIO_PMD_PMD12_OUTPUT, GPIO_PMD_PMD12_MASK);
	GPIO_Open(GPIOC, GPIO_PMD_PMD6_OUTPUT, GPIO_PMD_PMD6_MASK);

	while(1){
		GPIO_SetBit(GPIOB, 12);
		GPIO_ClrBit(GPIOC, 6);
		delay(749*3000);
		GPIO_ClrBit(GPIOB, 12);
		GPIO_SetBit(GPIOC, 6);
		delay(749*3000);
	}

	return 0;
}

inline void delay(unsigned int count) {
	volatile unsigned int i = count;

	// 7518 ==> 10ms delay
	//  749 ==>  1ms delay
	while (i--) {
		__asm__ volatile("nop");
	}
}
