#include <stdio.h>
#include <math.h>

#include "nano1xx.h"
#include "nano1xx_gpio.h"

#include "drv_i2c.h"
#include "drv_uart.h"

#include "sys_util.h"

#include "sensor_alt_BMP180.h"

#define MA_LEN	32

#define	BMP180_ADDR		0x77
#define	BMP180_CALIB	0xAA

#define	BMP180_ID		0xD0
#define	BMP180_ID_VAL	0x55

#define	BMP180_RESET	0xE0
#define	BMP180_CTRL		0xF4
#define	BMP180_ADC_OUT	0xF6

#define DELAY_TICKS		2

typedef struct {
	unsigned short	OSS;
	short			AC1;
	short			AC2;
	short			AC3;
	unsigned short	AC4;
	unsigned short	AC5;
	unsigned short	AC6;
	short			B1;
	short			B2;
	short			MB;
	short			MC;
	short			MD;
} BMP180_CALIB_STRUCT;

BMP180_CALIB_STRUCT	BMP180_Calib;

extern short temp_info_val;
extern short alte_info_val;
extern int sensor_port;
extern I2C_TypeDef *sensor_I2C;
extern volatile unsigned int current_ticks;
static volatile unsigned int pre_ticks;
static volatile unsigned int bmp180_status;

volatile unsigned  int ma_sum = 0;
volatile unsigned char ma_index = 0;
volatile unsigned  int ma_value[MA_LEN];

int ALT_BMP180_Init() {
	int i = 0;
	int ret = 0;
	unsigned char tmpval = 0;
	unsigned char *ptr = NULL;

#ifdef BMP180_DEBUG
	unsigned char msg[64];
#endif

	DRV_I2C_Init(sensor_port, 100000);
	DRV_I2C_Start(sensor_port);

	DRV_I2C_WriteRegByte(sensor_I2C, BMP180_ADDR, BMP180_CTRL, 0xB6);

	sys_delay(3000);

	ret = DRV_I2C_ReadRegByte(sensor_I2C, BMP180_ADDR, BMP180_ID);

#ifdef BMP180_DEBUG
	memset(msg, 0x00, 64);
	sprintf(msg, "BMP180 ID : 0x%02X\r\n", ret);
	DRV_UART_Send(msg, strlen(msg));
#endif

	if(ret != BMP180_ID_VAL) {
		ret = -1;

#ifdef BMP180_DEBUG
		memset(msg, 0x00, 64);
		sprintf(msg, "BMP180 ID is not valid!! Wrong Device\r\n");
		DRV_UART_Send(msg, strlen(msg));
#endif

		goto done;
	}

	ma_index = 0;
	ma_sum = 101325 * MA_LEN;
	for(i = 0; i < MA_LEN; i++)	ma_value[i] = 101325;

	memset(&BMP180_Calib, 0x00, sizeof(BMP180_CALIB_STRUCT));
	ptr = (unsigned char*)&(BMP180_Calib.AC1);
	ptr[0] = BMP180_CALIB;
	DRV_I2C_Recv(sensor_I2C, BMP180_ADDR, ptr, 22);

#if	__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	for(i = 0; i < 22; i = i + 2) {
		tmpval = ptr[i];
		ptr[i] = ptr[i + 1];
		ptr[i + 1] = tmpval;
	}

#endif

	BMP180_Calib.OSS = 0;

#ifdef BMP180_DEBUG
	memset(msg, 0x00, 16);
	for(i = 0; i < 22; i++) {
		sprintf(msg, "[%02d] 0x%02X\r\n", i, ptr[i]);
		DRV_UART_Send(msg, strlen(msg));
	}
#endif

	bmp180_status = 0;

	done:

	return ret;
}

int ALT_BMP180_Start() {
	pre_ticks = current_ticks;
	bmp180_status = 1;

	return 0;
}

int ALT_BMP180_Stop() {
	DRV_I2C_Stop(sensor_port);
	bmp180_status = 0;
	ma_sum = 0;
	ma_index = 0;
	memset(ma_value, 0x00, sizeof(unsigned int) * MA_LEN);

	return 0;
}

int ALT_BMP180_Reset() {
	int i = 0;
	int ret = 0;
	unsigned char tmpval = 0;
	unsigned char *ptr = NULL;

#ifdef BMP180_DEBUG
	unsigned char msg[64];
#endif

	DRV_I2C_WriteRegByte(sensor_I2C, BMP180_ADDR, BMP180_RESET, 0xB6);
	sys_delay(10000);

	DRV_I2C_WriteRegByte(sensor_I2C, BMP180_ADDR, BMP180_CTRL, 0xB6);
	sys_delay(3000);

	ret = DRV_I2C_ReadRegByte(sensor_I2C, BMP180_ADDR, BMP180_ID);

#ifdef BMP180_DEBUG
	memset(msg, 0x00, 64);
	sprintf(msg, "BMP180 ID : 0x%02X\r\n", ret);
	DRV_UART_Send(msg, strlen(msg));
#endif

	if(ret != BMP180_ID_VAL) {
		ret = -1;

#ifdef BMP180_DEBUG
		memset(msg, 0x00, 64);
		sprintf(msg, "BMP180 ID is not valid!! Wrong Device\r\n");
		DRV_UART_Send(msg, strlen(msg));
#endif

		goto done;
	}

	ma_index = 0;
	ma_sum = 101325 * MA_LEN;
	for(i = 0; i < MA_LEN; i++)	ma_value[i] = 101325;

	memset(&BMP180_Calib, 0x00, sizeof(BMP180_CALIB_STRUCT));
	ptr = (unsigned char*)&(BMP180_Calib.AC1);
	ptr[0] = BMP180_CALIB;
	DRV_I2C_Recv(sensor_I2C, BMP180_ADDR, ptr, 22);

#if	__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	for(i = 0; i < 22; i = i + 2) {
		tmpval = ptr[i];
		ptr[i] = ptr[i + 1];
		ptr[i + 1] = tmpval;
	}
#endif

	BMP180_Calib.OSS = 0;
	bmp180_status = 1;

done:
	return ret;
}

int ALT_BMP180_Set() {

	return 0;
}

int ALT_BMP180_Get() {
	int OSS = 0;
	int MA_P = 0;
	int P = 0;
	int UT = 0;
	int UP = 0;
	int X1 = 0;
	int X2 = 0;
	int X3 = 0;
	int B3 = 0;
	int B5 = 0;
	int B6 = 0;
	float p_val = 0.0;
	unsigned int B4 = 0;
	unsigned int B7 = 0;
	unsigned char i2c_buff[3];

#ifdef BMP180_DEBUG
	unsigned char msg[64];
#endif

	if(bmp180_status == 0) {
		return 0;
	} else {
		sys_delay(10000);
	}
	if((current_ticks - pre_ticks) < DELAY_TICKS)	return 0;

	pre_ticks = current_ticks;
	//GPIOC->DOUT ^= (1 << 6);
	//GPIOC->DOUT |= (1 << 6);

	OSS = BMP180_Calib.OSS;

	// get temperature reading
	i2c_buff[0] = BMP180_CTRL;
	i2c_buff[1] = 0x2E;
	DRV_I2C_Send(sensor_I2C, BMP180_ADDR, i2c_buff, 2);

	sys_delay(5000);

	i2c_buff[0] = BMP180_ADC_OUT;
	DRV_I2C_Recv(sensor_I2C, BMP180_ADDR, i2c_buff, 2);

	UT = (i2c_buff[0] << 8) + i2c_buff[1];

	// get pressure reading
	i2c_buff[0] = BMP180_CTRL;
	i2c_buff[1] = 0x34 + (OSS << 6);
	DRV_I2C_Send(sensor_I2C, BMP180_ADDR, i2c_buff, 2);

	sys_delay(1500 * ((1 << (OSS + 1)) + 1));

	i2c_buff[0] = BMP180_ADC_OUT;
	DRV_I2C_Recv(sensor_I2C, BMP180_ADDR, i2c_buff, 3);

	UP = ((i2c_buff[0] << 16) + (i2c_buff[1] << 8) + i2c_buff[2]) >> (8 - OSS);

	// both temperature and pressure take about 50us
	// calculate real temperature
	X1 = ((UT - BMP180_Calib.AC6) * BMP180_Calib.AC5) >> 15;
	X2 = ((BMP180_Calib.MC << 11) / (X1 + BMP180_Calib.MD));
	B5 = X1 + X2;
	temp_info_val = ((B5 + 8) >> 4);

	// calculate real pressure
	B6 = B5 - 4000;
	X1 = (BMP180_Calib.B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = (BMP180_Calib.AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((BMP180_Calib.AC1 << 2) + X3) << OSS) + 2) >> 2;
	X1 = (BMP180_Calib.AC3 * B6) >> 13;
	X2 = (BMP180_Calib.B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = (BMP180_Calib.AC4 * ((unsigned int)(X3 + 32768))) >> 15;
	B7 = ((unsigned int)(UP - B3)) * (50000 >> OSS);
	P = (B7 < 0x80000000) ? ((B7 << 1) / B4) : ((B7 / B4) << 1);
	X1 = (P >> 3) * (P >> 3);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * P) >> 16;
	P = P + ((X1 + X2 + 3791) >> 4);

	// pressure moving average
	ma_sum -= ma_value[ma_index];
	ma_sum += P;
	ma_value[ma_index] = P;
	ma_index++;
	ma_index = ma_index % MA_LEN;
	MA_P = ma_sum / MA_LEN;

	//GPIOC->DOUT |= (1 << 6);

	// calculate absolute altitude, take about 1.4ms
	p_val = 44330.0 * (1.0 - powf((float)((float)(MA_P) / 101325.0), 0.1902949572));
	alte_info_val = p_val > 0 ? (int) (p_val + 0.5) : (int) (p_val - 0.5);

	//GPIOC->DOUT &= ~(1 << 6);

#ifdef BMP180_DEBUG
	memset(msg, 0x00, 32);
	//sprintf(msg, "%04X %06X, %d %d %d %d %d\r\n", UT, UP, temp_info_val, P, alte_info_val, MA_P, ma_index);
	sprintf(msg, "%3d %6d\r\n", temp_info_val, alte_info_val);
	DRV_UART_Send(msg, strlen(msg));
#endif

	return 0;
}
