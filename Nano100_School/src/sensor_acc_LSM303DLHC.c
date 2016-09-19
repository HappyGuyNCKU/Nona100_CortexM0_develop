#include <stdio.h>

#include "nano1xx.h"
#include "nano1xx_gpio.h"

#include "drv_i2c.h"
#include "drv_uart.h"

#include "sys_info.h"

#include "sensor_acc_LSM303DLHC.h"

#define	LSM303DLHC_ADDR			0x19

#define LSM303DLHC_CTRL_REG1	0x20
#define LSM303DLHC_CTRL_REG2	0x21
#define LSM303DLHC_CTRL_REG3	0x22
#define LSM303DLHC_CTRL_REG4	0x23

#define LSM303DLHC_OUT_X_L_A	0x28
#define LSM303DLHC_OUT_X_H_A	0x29
#define LSM303DLHC_OUT_Y_L_A	0x2A
#define LSM303DLHC_OUT_Y_H_A	0x2B
#define LSM303DLHC_OUT_Z_L_A	0x2C
#define LSM303DLHC_OUT_Z_H_A	0x2D

#define LSM303DLHC_OUT_X_L_M	0x03
#define LSM303DLHC_OUT_X_H_M	0x04
#define LSM303DLHC_OUT_Y_L_M	0x05
#define LSM303DLHC_OUT_Y_H_M	0x06
#define LSM303DLHC_OUT_Z_L_M	0x07
#define LSM303DLHC_OUT_Z_H_M	0x08

#define LSM303DLHC_OUT_TEMP_L	0x31
#define LSM303DLHC_OUT_TEMP_H	0x32

extern unsigned short *acc_x_raw_buff;
extern unsigned short *acc_y_raw_buff;
extern unsigned short *acc_z_raw_buff;
extern Sensor_Info *acc_info_ptr;
extern int sensor_port;
extern I2C_TypeDef *sensor_I2C;

int ACC_LSM303DLHC_Init() {
	DRV_I2C_Init(sensor_port, 100000);
	DRV_I2C_Start(sensor_port);

	return 0;
}

int ACC_LSM303DLHC_Start() {
	DRV_I2C_Start(sensor_port);
	DRV_I2C_WriteRegByte(sensor_I2C, LSM303DLHC_ADDR, LSM303DLHC_CTRL_REG1, 0x47);
	DRV_I2C_WriteRegByte(sensor_I2C, LSM303DLHC_ADDR, LSM303DLHC_CTRL_REG4, 0x00);

	return 0;
}

int ACC_LSM303DLHC_Stop() {
	DRV_I2C_WriteRegByte(sensor_I2C, LSM303DLHC_ADDR, LSM303DLHC_CTRL_REG1, 0x00);
	DRV_I2C_Stop(sensor_I2C);

	return 0;
}

int ACC_LSM303DLHC_Set() {

	return 0;
}

int ACC_LSM303DLHC_Get() {
	unsigned int index = 0;
	unsigned char data[6];
	unsigned short val = 0;

#ifdef LSM303DLHC_DEBUG
	unsigned char msg[64];
#endif

	//GPIOC->DOUT ^= (1 << 6);
	//GPIOC->DOUT |= (1 << 6);

	//data[0] = LSM303DLHC_OUT_X_L_A;
	data[0] = 0xA8;
	DRV_I2C_Recv(sensor_I2C, LSM303DLHC_ADDR, data, 6);

	index = acc_info_ptr->raw_data_index;
	index++;
	index = index % ACC_RAW_MAX;

	acc_x_raw_buff[index] = (unsigned short)((data[1] << 8) | data[0]);
	acc_y_raw_buff[index] = (unsigned short)((data[3] << 8) | data[2]);;
	acc_z_raw_buff[index] = (unsigned short)((data[5] << 8) | data[4]);;

	acc_info_ptr->raw_data_index = index;

	//GPIOC->DOUT &= ~(1 << 6);

#ifdef LSM303DLHC_DEBUG
	memset(msg, 0x00, 32);
	sprintf(msg, "%04X %04X %04X\r\n", acc_x_raw_buff[index], acc_y_raw_buff[index], acc_z_raw_buff[index]);
	DRV_UART_Send(msg, strlen(msg));
#endif

	return 0;
}
