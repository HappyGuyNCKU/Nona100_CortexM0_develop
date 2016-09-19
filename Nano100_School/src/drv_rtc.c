#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nano1xx.h"
#include "nano1xx_rtc.h"
#include "nano1xx_sys.h"

#include "drv_rtc.h"
#include "drv_uart.h"

#include "sys_info.h"

extern Sensor_Hub_Info sh_info;
extern volatile unsigned char minute_up;
extern volatile unsigned char day_change;

int  DRV_RTC_Init() {
	S_DRVRTC_TIME_DATA_T rtcTime;

	memset(&rtcTime, 0x00, sizeof(S_DRVRTC_TIME_DATA_T));

	SYS_SetChipClockSrc((CLK_PWRCTL_HXT_EN | CLK_PWRCTL_LXT_EN), 1);
	//SYS_SetChipClockSrc((CLK_PWRCTL_HIRC_EN | CLK_PWRCTL_LIRC_EN), 1);

	while(SYS_CheckChipClockSrc(CLK_CLKSTATUS_LXT_STB | CLK_CLKSTATUS_HXT_STB) != 0);
	//while(SYS_CheckChipClockSrc(CLK_CLKSTATUS_HIRC_STB | CLK_CLKSTATUS_LIRC_STB) != 0);

	RTC_Init();

	// Time Setting
	rtcTime.u32Year 		= sh_info.year;
	rtcTime.u32cMonth 	    = sh_info.month;
	rtcTime.u32cDay 		= sh_info.day;
	rtcTime.u32cHour 		= sh_info.houre;
	rtcTime.u32cMinute 	    = sh_info.minute;
	rtcTime.u32cSecond 	    = sh_info.second;
	rtcTime.u32cDayOfWeek   = DRVRTC_FRIDAY;
	rtcTime.u8cClockDisplay = DRVRTC_CLOCK_24;

	// Initialization the RTC timer
	if(RTC_Open(&rtcTime) !=E_SUCCESS) return -1;

	return 0;
}

int  DRV_RTC_Start() {
	RTC_SetTickMode(DRVRTC_TICK_1_SEC);

	// Enable RTC Tick Interrupt
	RTC_EnableInt(RTC_RIER_TIER);

	return 0;
}

void DRV_RTC_Handler() {
	S_DRVRTC_TIME_DATA_T rtcTime;

	// Get the current time
	RTC_Read(&rtcTime);

	// check day change
	if(sh_info.day != rtcTime.u32cDay) day_change = 1;

	sh_info.year   = rtcTime.u32Year;
	sh_info.month  = rtcTime.u32cMonth;
	sh_info.day    = rtcTime.u32cDay;
	sh_info.houre  = rtcTime.u32cHour;
	sh_info.minute = rtcTime.u32cMinute;
	sh_info.second = rtcTime.u32cSecond;

	// check minute up
	if(sh_info.second == 0)	minute_up = 1;
}

int  DRV_RTC_Stop() {
	RTC_DisableInt(RTC_RIER_TIER);

	return 0;
}

int  DRV_RTC_Set() {
	S_DRVRTC_TIME_DATA_T rtcTime;

	memset(&rtcTime, 0x00, sizeof(S_DRVRTC_TIME_DATA_T));

	rtcTime.u32Year 		= sh_info.year;
	rtcTime.u32cMonth 	    = sh_info.month;
	rtcTime.u32cDay 		= sh_info.day;
	rtcTime.u32cHour 		= sh_info.houre;
	rtcTime.u32cMinute 	    = sh_info.minute;
	rtcTime.u32cSecond 	    = sh_info.second;
	rtcTime.u32cDayOfWeek   = DRVRTC_FRIDAY;
	rtcTime.u8cClockDisplay = DRVRTC_CLOCK_24;

	RTC_Write(&rtcTime);

	return 0;
}

int  DRV_RTC_Get() {
	S_DRVRTC_TIME_DATA_T rtcTime;

	RTC_Read(&rtcTime);
	sh_info.year   = rtcTime.u32Year;
	sh_info.month  = rtcTime.u32cMonth;
	sh_info.day    = rtcTime.u32cDay;
	sh_info.houre  = rtcTime.u32cHour;
	sh_info.minute = rtcTime.u32cMinute;
	sh_info.second = rtcTime.u32cSecond;

	return 0;
}

void RTC_IRQHandler(void) {
	if((RTC->RIER & RTC_RIER_TIER) && (RTC->RIIR & RTC_RIIR_TIS)) {
		RTC->RIIR = 0x2;		// tick interrupt occurred
		DRV_RTC_Handler();
	}
}
