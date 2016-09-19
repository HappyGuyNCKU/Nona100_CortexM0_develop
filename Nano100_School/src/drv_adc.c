#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nano1xx.h"
#include "nano1xx_adc.h"

#include "drv_adc.h"
#include "drv_uart.h"

#define ADC_CH0	0x00
#define ADC_CH1	0x01
#define ADC_CH2	0x02
#define ADC_CH3	0x03
#define ADC_CH4	0x04
#define ADC_CH5	0x05
#define ADC_CH6	0x06

int DRV_ADC_Init() {
	GCR->PA_L_MFP = PA0_MFP_ADC_CH0 |
					PA1_MFP_ADC_CH1 |
					PA2_MFP_ADC_CH2 |
					PA3_MFP_ADC_CH3 |
			        PA4_MFP_ADC_CH4 |
			        PA5_MFP_ADC_CH5 |
			        PA6_MFP_ADC_CH6;

	ADC_Init(ADC_CR_SINGLE_END, ADC_CR_ADMD_S_CYCLE, 0, ADC_CR_REFSEL_AVDD);

	ADC_SetChannelMask(ADC_CHER_CHEN_0 |
					   ADC_CHER_CHEN_1 |
					   ADC_CHER_CHEN_2 |
					   ADC_CHER_CHEN_3 |
					   ADC_CHER_CHEN_4 |
					   ADC_CHER_CHEN_5 |
					   ADC_CHER_CHEN_6);

  	ADC_SetResolution(ADC_CR_RESSEL_12BIT);

	return 0;
}

int DRV_ADC_Start() {

	return 0;
}

int DRV_ADC_Stop() {

	return 0;
}

int DRV_ADC_Set() {

	return 0;
}

int DRV_ADC_Get(unsigned int *data) {
	ADC_POWER_ON;
	ADC_START_CONV;

	while((ADC->SR & ADC_SR_ADF) == 0);
	ADC->SR = ADC_SR_ADF;

	ADC_STOP_CONV;

	data[0] = (ADC->RESULT6) & ADC_RESULT_RSLT_MASK; // SPO2 HR
	data[1] = (ADC->RESULT0) & ADC_RESULT_RSLT_MASK; // X-Axis, TP17
	data[2] = (ADC->RESULT1) & ADC_RESULT_RSLT_MASK; // Y-Axis, TP16
	data[3] = (ADC->RESULT3) & ADC_RESULT_RSLT_MASK; // Z-Axis, TP15
	data[4] = (ADC->RESULT4) & ADC_RESULT_RSLT_MASK; // Battery Voltage
	data[5] = (ADC->RESULT2) & ADC_RESULT_RSLT_MASK;
	data[6] = (ADC->RESULT5) & ADC_RESULT_RSLT_MASK;

	return 0;
}
