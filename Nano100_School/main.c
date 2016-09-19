#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "drv_adc.h"
#include "drv_gpio.h"
#include "drv_uart.h"
#include "drv_i2c.h"
#include "drv_int.h"
#include "drv_rtc.h"
#include "drv_timer.h"
#include "drv_bt_SBC2112.h"
#include "drv_display_SSD1306.h"
#include "drv_flash_SST25VF032B.h"

#include "sys_ui.h"
#include "sys_info.h"
#include "sys_init.h"
#include "sys_util.h"
#include "sys_shell.h"

#include "sensor_spo2.h"
#include "sensor_alt_BMP180.h"
#include "sensor_acc_LSM303DLHC.h"

#include "algo_pedo_r4.h"
#include "algo_spo2hr.h"

extern volatile int bt_status;
extern volatile unsigned int sensorhub_state;
extern volatile unsigned char timer0_status;

int main(void) {
	int i = 0;
	char msg[64];
	unsigned char buff[16];

#ifdef BLE_DEBUG
	unsigned char buff[256];
#endif

	MCU_Initial();
	MCU_Boot();

	// initial whole system data storage
	shell_open();

	DRV_GPIO_Init();
	DRV_UART_Init();
	DRV_INT_Init();
	DRV_ADC_Init();
	DRV_TIMER_Init(TIMER_0, 20000);
	DRV_TIMER_Init(TIMER_1, 1000000);
	DRV_RTC_Init();

	memset(msg, 0x00, 64);
	sprintf(msg, "\r\nSensor Hub Booting....\r\n");
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "\r\nSensor Hub FW:%s\r\n\r\n", FW_VERSION);
	DRV_UART_Send(msg, strlen(msg));

	DRV_BT_SBC2112_Init();
	DRV_BT_SBC2112_Stop();

	// Flash on SPI-0
	DRV_FLASH_SST25VF032B_Init();
	DRV_FLASH_SST25VF032B_Start();

	DRV_FLASH_SST25VF032B_DisableProtection();
	for(i = 0; i < DATA_PAGE_COUNT; i++) {
		DRV_FLASH_SST25VF032B_4KErase(DATA_PAGE_BASE + i * PAGE_SIZE);
		// should delay after every sector erase
		sys_delay(16930);
	}
	DRV_FLASH_SST25VF032B_EnableProtection();

	// Display on SPI-1
	DRV_DISPLAY_SSD1306_Init();
	DRV_DISPLAY_SSD1306_Start();
	DRV_DISPLAY_SSD1306_Clear();
	DRV_DISPLAY_SSD1306_On();

	// initial sensor devices
	SPO2_Init();
	SPO2_Start();

	ALT_BMP180_Init();
	ALT_BMP180_Start();

	ACC_LSM303DLHC_Init();
	ACC_LSM303DLHC_Start();

	// initial algorithm
	spo2HRDect_open();
	pedoDect_R4_open();

	DRV_RTC_Start();
	DRV_TIMER_Start(TIMER_0);
	DRV_TIMER_Start(TIMER_1);
	DRV_INT_Start();

	sensorhub_state = STATE_LOGO;

	while(1) {
		while(!timer0_status) sys_delay(749);
		timer0_status = 0;

		if(bt_status == SBC2112_READY) {
			while(1) {
				if(bt_status == SBC2112_DISCONNECT || bt_status == SBC2112_UNKNOWN)	break;
				if(DRV_BT_SBC2112_Wait_Connect() == SBC2112_CONNECTED)	break;
			}

			while(1) {
				if(bt_status == SBC2112_DISCONNECT || bt_status == SBC2112_UNKNOWN)	break;
				sys_delay(7490);
			}

			ALT_BMP180_Reset();

		} else {
			bt_status = SBC2112_DISCONNECT;
			ALT_BMP180_Get();
			ACC_LSM303DLHC_Get();
		}
	}

	return 0;
}
