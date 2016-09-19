#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nano1xx.h"
#include "nano1xx_gpio.h"

#include "drv_int.h"
#include "drv_uart.h"
#include "drv_timer.h"
#include "drv_bt_SBC2112.h"
#include "drv_display_SSD1306.h"

#include "sys_ui.h"
#include "sys_info.h"

#define DEBOUNCE_TICKS	16

extern volatile unsigned int current_ticks;
extern volatile unsigned int sensorhub_state;
extern volatile int timer0_activate;
extern volatile int timer1_activate;
extern volatile int timer2_activate;
extern volatile int timer3_activate;
extern volatile unsigned int backlight_off_counts;
extern volatile unsigned int display_refresh_tick;
extern volatile unsigned int ble_connection_tick;

extern volatile unsigned int display_on;
extern volatile int bt_status;

static volatile unsigned int pre_ticks;
static volatile int direction = 0;

int DRV_INT_Init() {
	GPIO_SetDebounceTime(GPIO_DBNCECON_DBCLKSEL_1, GPIO_DBNCECON_DBCLKSRC_HCLK);

	// For Nano130 POC Board
	//GPIO_EnableDebounce(GPIOC, 12);

	// For Nano100 POC Board
	GPIO_EnableDebounce(GPIOB, 8);
	GPIO_EnableDebounce(GPIOB, 15);

	pre_ticks = current_ticks;

	return 0;
}

int DRV_INT_Start() {
	MFP_EXT_INT0_TO_PB8();
	GPIO_EnableEINT0(GPIOB, 8, GPIO_IER_IF_EN_8, GPIO_IMD_EDGE_8);

	MFP_EXT_INT1_TO_PB15();
	GPIO_EnableEINT1(GPIOB, 15, GPIO_IER_IF_EN_15, GPIO_IMD_EDGE_15);

	pre_ticks = current_ticks;

	return 0;
}

int DRV_INT_Handler() {
	char intmsg[64];
	int change_state = 0;

	memset(intmsg, 0x00, 64);
	if(timer1_activate == TIMER_OFF) {
		//sensorhub_state = 0;
		sprintf(intmsg, "\r\ntimer 1 turn on to control display....\r\n");
		DRV_UART_Send(intmsg, strlen(intmsg));
		DRV_TIMER_Init(TIMER_1, 1000000);
		DRV_TIMER_Start(TIMER_1);
	}

	memset(intmsg, 0x00, 64);

	display_refresh_tick = 0;

	sys_ui_clear();

	if(display_on == 0) {
		DRV_DISPLAY_SSD1306_On();
		display_on = 1;
	}

	if(direction == 1) {
		sensorhub_state++;
	} else {
		sensorhub_state--;
		if(sensorhub_state == -1) sensorhub_state = (STATE_LAST - 1);
	}

	sensorhub_state = sensorhub_state % STATE_LAST;

	switch(sensorhub_state) {
		case STATE_LOGO:
			sprintf(intmsg, "\r\ncurrent state: logo info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_logo();
			break;

		case STATE_DATE:
			sprintf(intmsg, "\r\ncurrent state: date info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_date();
			break;

		case STATE_TIME:
			sprintf(intmsg, "\r\ncurrent state: time info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_time();
			break;

		case STATE_PEDO:
			sprintf(intmsg, "\r\ncurrent state: pedo info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_pedo();
			break;

		case STATE_TEMP:
			sprintf(intmsg, "\r\ncurrent state: temp info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_temp();
			break;

		case STATE_HR:
			sprintf(intmsg, "\r\ncurrent state:  hr  info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS * 2;
			sys_ui_hr();
			break;

		case STATE_ALT:
			sprintf(intmsg, "\r\ncurrent state:  alt  info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_alt();
			break;

		case STATE_BLE_MAC:
			sprintf(intmsg, "\r\ncurrent state:  ble mac info.\r\n");
			change_state = 1;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			break;

		case STATE_BLE:
			change_state = 1;
			//backlight_off_counts = BACKLIGHT_OFF_COUNTS << 3;
			backlight_off_counts = BACKLIGHT_OFF_COUNTS;
			sys_ui_bleconn();
			if(timer3_activate == TIMER_OFF) {
				sprintf(intmsg, "\r\ntimer 3 turn on to control BLE....\r\n");
				DRV_UART_Send(intmsg, strlen(intmsg));
				//if(bt_status != SBC2112_READY) DRV_BT_SBC2112_Init();
				DRV_BT_SBC2112_Init();
				DRV_TIMER_Init(TIMER_3, 1000000);
				DRV_TIMER_Start(TIMER_3);
			}
			sprintf(intmsg, "\r\ncurrent state: BLE Enable.....\r\n");
			break;

		default:
			change_state = 0;
			break;
	}

	sys_ui_update();

	if(change_state == 1) DRV_UART_Send(intmsg, strlen(intmsg));
	if(sensorhub_state != STATE_BLE)	ble_connection_tick = backlight_off_counts << 2;

	return 0;
}

int DRV_INT_Stop() {
	GPIO_DisableEINT0(GPIOB, 8);

	return 0;
}

// External INT0 ISR
void EINT0_IRQHandler(void) {
	// EINT0 (GPB8) Clear the interrupt
	GPIOB->ISR = 1 << 8;

	direction = 1;
	if((current_ticks - pre_ticks) > DEBOUNCE_TICKS) {
		pre_ticks = current_ticks;
		DRV_INT_Handler();
	}
}

// External INT1 ISR
void EINT1_IRQHandler(void) {
	//EINT1 (GPB15) Clear the interrupt
	GPIOB->ISR = 1 << 15;

	direction = 0;
	if((current_ticks - pre_ticks) > DEBOUNCE_TICKS) {
		pre_ticks = current_ticks;
		DRV_INT_Handler();
	}
}
