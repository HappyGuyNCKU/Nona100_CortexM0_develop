#include <stdio.h>

#include "drv_i2c.h"
#include "drv_uart.h"

#define	I2C_ERROR_OK			 0
#define	I2C_ERROR_PROTOCOL		-1
#define	I2C_ERROR_TIMEOUT		-2
#define	I2C_ERROR_PROCESSING	-3

//#define I2C_DEBUG

typedef enum {
	i2c_write = 0,
	i2c_read = 1,
};

static struct I2CStruct {
	volatile int errval;
	int devaddr;
	int rwdir;
	unsigned char *dataptr;
	unsigned char *dataend;
} i2c0, i2c1;

static int I2C0_Status = I2C_UNKOWN;
static int I2C1_Status = I2C_UNKOWN;

int sensor_port = PORT_I2C0;
I2C_TypeDef *sensor_I2C = I2C0;

int DRV_I2C_Init(int i2c_port, int speed) {
	int ret = I2C_UNKOWN;

	switch(i2c_port) {
		case PORT_I2C0:
			if(I2C0_Status == I2C_READY) {
				ret = I2C_READY;
				goto done;
			}

			// Set multi function pin for I2C0
	    	GCR->PA_H_MFP = (GCR->PA_H_MFP & ~(PA8_MFP_MASK | PA9_MFP_MASK)) | (PA8_MFP_I2C0_SDA | PA9_MFP_I2C0_SCL);
			I2C_SetTimeoutCounter(I2C0, 1, 0);
			I2C_Init(I2C0);
			I2C_Open(I2C0, speed);
			I2C_EnableInt(I2C0);

			I2C0_Status = I2C_READY;
			ret = I2C_READY;
		    break;

		case PORT_I2C1:
			if(I2C1_Status == I2C_READY) {
				ret = I2C_READY;
				goto done;
			}
			// Set multi function pin for I2C1
	    	GCR->PA_H_MFP = (GCR->PA_H_MFP & ~(PA10_MFP_MASK|PA11_MFP_MASK)) | (PA10_MFP_I2C1_SDA | PA11_MFP_I2C1_SCL);
	    	I2C_SetTimeoutCounter(I2C1, 1, 0);
	    	I2C_Init(I2C1);
			I2C_Open(I2C1, speed);
			I2C_EnableInt(I2C1);

			I2C1_Status = I2C_READY;
			ret = I2C_READY;
			break;

		default:
			break;
	}

done:

	return ret;
}

int DRV_I2C_Start(int i2c_port) {
	int ret = I2C_UNKOWN;

	switch(i2c_port) {
		case PORT_I2C0:
			if(I2C0_Status != I2C_READY) goto done;
			I2C_EnableInt(I2C0);

			I2C0_Status = I2C_READY;
			ret = I2C_READY;
		    break;

		case PORT_I2C1:
			if(I2C1_Status != I2C_READY) goto done;
			I2C_EnableInt(I2C1);

			I2C1_Status = I2C_READY;
			ret = I2C_READY;
			break;

		default:
			break;
	}

done:

	return ret;
}

int DRV_I2C_Stop(int i2c_port) {
	int ret = 0;

	switch(i2c_port) {
		case PORT_I2C0:
			if(I2C0_Status == I2C_UNKOWN)	goto done;

			// Disable I2C0 interrupt and clear corresponding NVIC bit
			I2C_DisableInt(I2C0);

			// Close I2C0
			I2C_Close(I2C0);

			I2C_DeInit(I2C0);
			I2C0_Status = I2C_UNKOWN;
			break;

		case PORT_I2C1:
			if(I2C1_Status == I2C_UNKOWN)	goto done;

			// Disable I2C1 interrupt and clear corresponding NVIC bit
			I2C_DisableInt(I2C1);

			// Close I2C1
			I2C_Close(I2C1);

			I2C_DeInit(I2C1);
			I2C1_Status = I2C_UNKOWN;
			break;

		default:
			break;
	}

done:

	return ret;
}

int DRV_I2C_Send(I2C_TypeDef *I2CPort, unsigned char addr, unsigned char *buf, int len) {
	struct I2CStruct *i2c = (I2CPort == I2C0) ? &i2c0 : &i2c1;

	i2c->devaddr = addr;
	i2c->dataptr = buf;
	i2c->dataend = buf + len;
	i2c->rwdir = i2c_write;
	i2c->errval = I2C_ERROR_PROCESSING;
	I2C_Ctrl(I2CPort, 1, 0, 0);
	while (i2c->errval == I2C_ERROR_PROCESSING);

	return i2c->errval;
}

int DRV_I2C_Recv(I2C_TypeDef *I2CPort, unsigned char addr, unsigned char *buf, int len) {
	struct I2CStruct *i2c = (I2CPort == I2C0) ? &i2c0 : &i2c1;

	i2c->devaddr = addr;
	i2c->dataptr = buf;
	i2c->dataend = buf + len - 1;
	i2c->rwdir = i2c_read;
	i2c->errval = I2C_ERROR_PROCESSING;
	I2C_Ctrl(I2CPort, 1, 0, 0);
	while (i2c->errval == I2C_ERROR_PROCESSING);

	return i2c->errval;
}

int  DRV_I2C_ReadRegByte(I2C_TypeDef *I2CPort, unsigned char devAddr, unsigned char regAddr) {
	DRV_I2C_Recv(I2CPort, devAddr, &regAddr, 1);

	return regAddr;
}

int DRV_I2C_WriteRegByte(I2C_TypeDef *I2CPort, unsigned char devAddr, unsigned char regAddr, unsigned char data) {
	unsigned char buf[2] = { regAddr, data };

	DRV_I2C_Send(I2CPort, devAddr, buf, 2);

	return 0;
}

int DRV_I2C_Handler() {

	return 0;
}

void I2C0_IRQHandler(void) {
    unsigned int status;

    // clear interrupt flag
    I2C0->INTSTS |= I2C_INTSTS_INTSTS;

	status = I2C0->STATUS;

	if (I2C0->INTSTS & I2C_INTSTS_TIF) {
		I2C0->INTSTS |= I2C_INTSTS_TIF;	// Clear TIF
		i2c0.errval = I2C_ERROR_TIMEOUT;
		return;
	}

	if (status == 0x08) {					// START has been transmitted
		I2C_WriteData(I2C0, i2c0.devaddr << 1);
		I2C_Trig(I2C0);
	} else if (status == 0x10) {			// Repeat START has been transmitted and prepare SLA+R
		I2C_WriteData(I2C0, (i2c0.devaddr << 1) | 0x01);
		I2C_Trig(I2C0);
	} else if (status == 0x18) {			// SLA+W has been transmitted and ACK has been received
		I2C_WriteData(I2C0, *i2c0.dataptr);
		I2C_Trig(I2C0);
		if (i2c0.rwdir == i2c_write)	i2c0.dataptr++;
	} else if (status == 0x20) {			// SLA+W has been transmitted and NACK has been received
		I2C_Ctrl(I2C0, 1, 1, 0);
		I2C_Trig(I2C0);
	}else if (status == 0x28) {				// DATA has been transmitted and ACK has been received
		if (i2c0.rwdir == i2c_read)	{
			I2C_Ctrl(I2C0, 1, 0, 0);
		} else if (i2c0.dataptr < i2c0.dataend) {
			I2C_WriteData(I2C0, *(i2c0.dataptr++));
		} else {
			I2C_Ctrl(I2C0, 0, 1, 0);		// Stop
			i2c0.errval = I2C_ERROR_OK;
		}
		I2C_Trig(I2C0);
	} else if (status == 0x40) {			// SLA+R has been transmitted and ACK has been received
		I2C_Ctrl(I2C0, 0, 0, i2c0.dataptr < i2c0.dataend);
		I2C_Trig(I2C0);
	} else if (status == 0x50) {			// DATA has been received and ACK has been returned
		*(i2c0.dataptr++) = I2C_ReadData(I2C0);
		I2C_Ctrl(I2C0, 0, 0, i2c0.dataptr < i2c0.dataend);
		I2C_Trig(I2C0);
	} else if (status == 0x58) {			// DATA has been received and NACK has been returned
		*i2c0.dataptr = I2C_ReadData(I2C0);
		I2C_Ctrl(I2C0, 0, 1, 0);
		I2C_Trig(I2C0);
		i2c0.errval = I2C_ERROR_OK;
	} else if (status == 0xF8) {			// Stop
		I2C_Ctrl(I2C0, 0, 1, 0);
	} else {
		// unhandled
		i2c0.errval = I2C_ERROR_PROTOCOL;
	}
}

void I2C1_IRQHandler(void) {
    unsigned int status;

    // clear interrupt flag
    I2C1->INTSTS |= I2C_INTSTS_INTSTS;

	status = I2C1->STATUS;

	if (I2C1->INTSTS & I2C_INTSTS_TIF) {
		I2C1->INTSTS |= I2C_INTSTS_TIF;	// Clear TIF
		i2c1.errval = I2C_ERROR_TIMEOUT;
		return;
	}

	if (status == 0x08) {					// START has been transmitted
		I2C_WriteData(I2C1, i2c1.devaddr << 1);
		I2C_Trig(I2C1);
	} else if (status == 0x10) {			// Repeat START has been transmitted and prepare SLA+R
		I2C_WriteData(I2C1, (i2c1.devaddr << 1) | 0x01);
		I2C_Trig(I2C1);
	} else if (status == 0x18) {			// SLA+W has been transmitted and ACK has been received
		I2C_WriteData(I2C1, *i2c1.dataptr);
		I2C_Trig(I2C1);
		if (i2c1.rwdir == i2c_write)	i2c1.dataptr++;
	} else if (status == 0x20) {			// SLA+W has been transmitted and NACK has been received
		I2C_Ctrl(I2C1, 1, 1, 0);
		I2C_Trig(I2C1);
	}else if (status == 0x28) {				// DATA has been transmitted and ACK has been received
		if (i2c1.rwdir == i2c_read)	{
			I2C_Ctrl(I2C1, 1, 0, 0);
		} else if (i2c1.dataptr < i2c1.dataend) {
			I2C_WriteData(I2C1, *(i2c1.dataptr++));
		} else {
			I2C_Ctrl(I2C1, 0, 1, 0);		// Stop
			i2c1.errval = I2C_ERROR_OK;
		}
		I2C_Trig(I2C1);
	} else if (status == 0x40) {			// SLA+R has been transmitted and ACK has been received
		I2C_Ctrl(I2C1, 0, 0, i2c1.dataptr < i2c1.dataend);
		I2C_Trig(I2C1);
	} else if (status == 0x50) {			// DATA has been received and ACK has been returned
		*(i2c1.dataptr++) = I2C_ReadData(I2C1);
		I2C_Ctrl(I2C1, 0, 0, i2c1.dataptr < i2c1.dataend);
		I2C_Trig(I2C1);
	} else if (status == 0x58) {			// DATA has been received and NACK has been returned
		*i2c1.dataptr = I2C_ReadData(I2C1);
		I2C_Ctrl(I2C1, 0, 1, 0);
		I2C_Trig(I2C1);
		i2c1.errval = I2C_ERROR_OK;
	} else if (status == 0xF8) {			// Stop
		I2C_Ctrl(I2C1, 0, 1, 0);
	} else {
		// unhandled
		i2c1.errval = I2C_ERROR_PROTOCOL;
	}
}

