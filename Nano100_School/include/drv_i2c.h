#ifndef DRV_I2C_H
#define DRV_I2C_H

#include "nano1xx.h"
#include "nano1xx_i2c.h"

typedef enum {
	PORT_I2C0 = 0,
	PORT_I2C1 = 1
} port_i2c_num;

typedef enum {
	I2C_UNKOWN = -1,
	I2C_READY = 0,
	I2C_BUSY = 1,
} port_i2c_status;

int DRV_I2C_Init(int i2c_port, int speed);
int DRV_I2C_Start(int i2c_port);
int DRV_I2C_Stop(int i2c_port);

int DRV_I2C_Recv(I2C_TypeDef *I2CPort, unsigned char addr, unsigned char *buf, int len);
int DRV_I2C_Send(I2C_TypeDef *I2CPort, unsigned char addr, unsigned char *buf, int len);

int DRV_I2C_ReadRegByte(I2C_TypeDef *I2CPort, unsigned char devAddr, unsigned char regAddr);
int DRV_I2C_WriteRegByte(I2C_TypeDef *I2CPort, unsigned char devAddr, unsigned char regAddr, unsigned char data);

int DRV_I2C_Handler();

#endif
