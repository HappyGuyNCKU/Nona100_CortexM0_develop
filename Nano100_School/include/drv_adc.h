#ifndef DRV_ADC_H
#define DRV_ADC_H

int DRV_ADC_Init();
int DRV_ADC_Start();
int DRV_ADC_Set();
int DRV_ADC_Get(unsigned int *data);
int DRV_ADC_Stop();

#endif
