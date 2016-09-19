#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#define BACKLIGHT_OFF_COUNTS	60
#define MINUTE_PER_DAY			(60 * 24)

typedef enum {
	TIMER_0 = 0,
	TIMER_1,
	TIMER_2,
	TIMER_3
} Timer_NO;

typedef enum {
	TIMER_OFF = -1,
	TIMER_ON = 1
} Timer_Status;

typedef void (*INFO_UPDATE_FUNC) (void);

int DRV_TIMER_Init(int timer_no, int clock); // min unit : 1 * 10^-6 second.

int DRV_TIMER_Start(int timer_no);

int DRV_TIMER_Stop(int timer_no);

int DRV_TIMER_Close(int timer_no);

int DRV_TIMER0_Handler();

int DRV_TIMER1_Handler();

int DRV_TIMER2_Handler();

int DRV_TIMER3_Handler();

#endif
