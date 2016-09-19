#include "nano1xx.h"
#include "nano1xx_timer.h"
#include "nano1xx_uart.h"

#include "drv_adc.h"
#include "drv_timer.h"
#include "drv_uart.h"
#include "drv_bt_SBC2112.h"

#include "sys_info.h"
#include "sys_ui.h"
#include "sys_util.h"

#include "sensor_acc_LSM303DLHC.h"
#include "sensor_spo2.h"

#include "algo_spo2hr.h"
#include "algo_pedo_r4.h"

#define MINUTE_PER_DAY	(60 * 24)

#define BAT_VOL_HIGH	3900
#define BAT_VOL_LOW		3400
#define BAT_VOL_STEP	(BAT_VOL_HIGH - BAT_VOL_LOW)

volatile unsigned int backlight_off_counts = BACKLIGHT_OFF_COUNTS;
volatile unsigned int display_on = 0;
volatile unsigned char minute_up = 0;
volatile unsigned char day_change = 0;
volatile unsigned char timer0_status = 0;

volatile unsigned int pre_minute = 0;
volatile unsigned int current_ticks = 0;
volatile unsigned int display_refresh_tick = 0;
volatile unsigned int ble_connection_tick = 0;
volatile unsigned int adc_data[7];
volatile unsigned int pre_second = 0;

volatile unsigned int bat_index;
volatile unsigned int bat_value[8];
volatile unsigned int bat_sum;

volatile int timer0_activate = TIMER_OFF;
volatile int timer1_activate = TIMER_OFF;
volatile int timer2_activate = TIMER_OFF;
volatile int timer3_activate = TIMER_OFF;

extern Sensor_Hub_Info sh_info;
extern Sensor_Info *ecg_info_ptr;
extern Sensor_Info *temp_info_ptr;
extern Sensor_Info *acc_info_ptr;
extern Sensor_Info *alt_info_ptr;

extern Data_Storage Storage;

extern unsigned  int  pedo_info_buff[];
extern unsigned  char hr_info_buff[];
extern 		    short temp_info_buff[];
extern          short alt_info_buff[];
extern unsigned short sys_tick_buff[];
extern unsigned short current_sys_tick;

extern unsigned int  pedo_info_val;
extern unsigned char hr_info_val;
extern 		   short temp_info_val;
extern         short alte_info_val;

extern unsigned short *acc_x_raw_buff;
extern unsigned short *acc_y_raw_buff;
extern unsigned short *acc_z_raw_buff;

extern volatile int bt_status;
extern volatile unsigned int battery_capacity;
extern volatile unsigned int sensorhub_state;

extern UART_TypeDef *con_port;
extern UART_TypeDef *ble_port;

int DRV_TIMER_Init(int timer_no, int clock) {
	//TIMER_Init(TIMER0, 11, 1000000, TIMER_CTL_MODESEL_PERIODIC); //   1Hz
	//TIMER_Init(TIMER0, 11,   20000, TIMER_CTL_MODESEL_PERIODIC); //  50Hz
	//TIMER_Init(TIMER0, 11,    2000, TIMER_CTL_MODESEL_PERIODIC); // 500Hz

	if(clock <= 0)	return -1;

	switch(timer_no) {
	case TIMER_0:	// for ADC Sampling
		current_ticks = 0;
		timer0_status = 0;
		TIMER_Init(TIMER0, 11,   clock, TIMER_CTL_MODESEL_PERIODIC);
		break;

	case TIMER_1:	// for display refresh tick and back light control
		display_refresh_tick = 0;
		TIMER_Init(TIMER1, 11,   clock, TIMER_CTL_MODESEL_PERIODIC);
		break;

	case TIMER_2:
		TIMER_Init(TIMER2, 11,   clock, TIMER_CTL_MODESEL_PERIODIC);
		break;

	case TIMER_3:	// for BLE Control
		ble_connection_tick = 0;
		TIMER_Init(TIMER3, 11,   clock, TIMER_CTL_MODESEL_PERIODIC);
		break;

	default:
		break;
	}

	return 0;
}

int DRV_TIMER_Start(int timer_no) {
	switch(timer_no) {
	case TIMER_0:
		minute_up = 0;
		pre_second = 0;
		bat_sum = 0;
		bat_index = 0;
		memset(adc_data, 0x00, 7 * sizeof(unsigned int));
		memset(bat_value, 0x00, 8 * sizeof(unsigned int));
		timer0_activate = TIMER_ON;
		timer0_status = 0;
		TIMER_EnableInt(TIMER0, TIMER_IER_TMRIE);
		TIMER_Start(TIMER0);
		break;

	case TIMER_1:
		timer1_activate = TIMER_ON;
		display_refresh_tick = 0;
		TIMER_EnableInt(TIMER1, TIMER_IER_TMRIE);
		TIMER_Start(TIMER1);
		break;

	case TIMER_2:
		timer2_activate = TIMER_ON;
		TIMER_EnableInt(TIMER2, TIMER_IER_TMRIE);
		TIMER_Start(TIMER2);
		break;

	case TIMER_3:
		timer3_activate = TIMER_ON;
		ble_connection_tick = 0;
		TIMER_EnableInt(TIMER3, TIMER_IER_TMRIE);
		TIMER_Start(TIMER3);
		break;

	default:
		break;
	}

	return 0;
}

int DRV_TIMER_Stop(int timer_no) {
	switch(timer_no) {
	case TIMER_0:
		timer0_activate = TIMER_OFF;
		timer0_status = 0;
		TIMER_DisableInt(TIMER0, TIMER_IER_TMRIE);
		break;

	case TIMER_1:
		timer1_activate = TIMER_OFF;
		TIMER_DisableInt(TIMER1, TIMER_IER_TMRIE);
		break;

	case TIMER_2:
		timer2_activate = TIMER_OFF;
		TIMER_DisableInt(TIMER2, TIMER_IER_TMRIE);
		break;

	case TIMER_3:
		timer3_activate = TIMER_OFF;
		TIMER_DisableInt(TIMER3, TIMER_IER_TMRIE);
		break;

	default:
		break;
	}

	return 0;
}

int DRV_TIMER_Close(int timer_no) {
	switch(timer_no) {
	case TIMER_0:
		TIMER_DeInit(TIMER0);
		break;

	case TIMER_1:
		TIMER_DeInit(TIMER1);
		break;

	case TIMER_2:
		TIMER_DeInit(TIMER2);
		break;

	case TIMER_3:
		TIMER_DeInit(TIMER3);
		break;

	default:
		break;
	}

	return 0;
}

int DRV_TIMER0_Handler() {
	char intmsg[64];
	int bat_tmp_cap = 0;
	unsigned int pedo_val = 0;
	unsigned int spo2hr_val = 0;
	unsigned int tmp_index = 0;
	unsigned int write_address = 0;
	Data_Record record_data, read_data;

	//GPIOC->DOUT ^= (1 << 6);
	//GPIOE->DOUT ^= (1 << 6);

	//GPIOC->DOUT |= (1 << 6);
	//GPIOE->DOUT |= (1 << 6);

	current_ticks++;

	DRV_ADC_Get(adc_data);
	spo2hr_val = spo2HRDect_process(adc_data[0]);
	spo2hr_val = spo2HRDect_detection(spo2hr_val);
	hr_info_val = (spo2hr_val) & 0xFF;

	//	GPIOC->DOUT |= (1 << 6);
	pedo_val = pedoDect_R4_process(adc_data[1], adc_data[2], adc_data[3]);
	pedo_info_val = pedoDect_R4_detection(pedo_val);
	//	GPIOC->DOUT &= ~(1 << 6);

	if(pre_second != sh_info.second) {
		pre_second = sh_info.second;

		bat_sum -= bat_value[bat_index];
		bat_sum += adc_data[4];
		bat_value[bat_index] = adc_data[4];
		bat_index++;
		bat_index = bat_index & 0x07;

		// update battery voltage value
		//sh_info.battery_voltage_work = adc_data[4];

		// must add some voltage to capacity mapping code
		bat_tmp_cap = bat_sum >> 3;
		if(bat_tmp_cap > BAT_VOL_HIGH) {
			battery_capacity = 100;
		} else if(bat_tmp_cap < BAT_VOL_LOW){
			battery_capacity = 0;
		} else {
			battery_capacity = (100 * (bat_tmp_cap - BAT_VOL_LOW)) / BAT_VOL_STEP;
		}

#if 0
		if(bat_index == 7) {
			memset(intmsg, 0x00, 64);
			sprintf(intmsg,
					"[%02d:%02d:%02d] %4d %4d %3d\r\n",
					sh_info.houre,
					sh_info.minute,
					sh_info.second,
					adc_data[4],
					bat_sum >> 3,
					battery_capacity);
			DRV_UART_Send(intmsg, strlen((const char *)intmsg));
		}
#endif
	}

	if(minute_up == 1) {
		minute_up = 0;

		// check day change
		if (day_change) {
			Storage.current_day = (Storage.current_day + 1) % RECORD_DAY_MAX;
			memset(Storage.record_day[Storage.current_day].date, 0x00, 12);
			sprintf(Storage.record_day[Storage.current_day].date, "%04d-%02d-%02d", sh_info.year, sh_info.month, sh_info.day);
			Storage.record_day[Storage.current_day].current_index = 0;
			Storage.record_day[Storage.current_day].total_entries = 0;
		}

		// get current_index from current time in sh_info
		tmp_index = sh_info.houre * 60 + sh_info.minute;

		// record current_index in the record_day
		Storage.record_day[Storage.current_day].current_index = tmp_index;

		// calculate the write address in flash
		write_address = Storage.record_day[Storage.current_day].begin_address + (Storage.record_day[Storage.current_day].current_index * sizeof(Data_Record));

		tmp_index = (tmp_index % SYS_TICK_MAX);
		current_sys_tick = tmp_index;
		sys_tick_buff[tmp_index] = sh_info.houre * 60 + sh_info.minute;

		ecg_info_ptr->info_data_index = tmp_index;
		hr_info_buff[tmp_index] = spo2hr_val;

		acc_info_ptr->info_data_index = tmp_index;
		pedo_info_buff[tmp_index] = pedo_info_val;

		temp_info_ptr->info_data_index = tmp_index;
		temp_info_buff[tmp_index] = temp_info_val;

		alt_info_ptr->info_data_index = tmp_index;
		alt_info_buff[tmp_index] = alte_info_val;

		// write data into flash
		record_data.pedo_counts = pedo_info_val;
		record_data.altitude    = alte_info_val;
		record_data.temperature = temp_info_val;
		record_data.heart_rate  = hr_info_val;
		sys_data_write(write_address, &record_data, sizeof(Data_Record));

		// record total_entries in the record_day
		Storage.record_day[Storage.current_day].total_entries++;

#if 0
		memset(&read_data, 0x00, sizeof(Data_Record));
		sys_data_read(write_address, &read_data, sizeof(Data_Record));
		memset(intmsg, 0x00, 64);
		sprintf(intmsg,	"read data: pedo=%d alt=%d temp=%d hr=%d \r\n",read_data.pedo_counts,read_data.altitude,read_data.temperature,
				read_data.heart_rate);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));

		memset(intmsg, 0x00, 64);
		sprintf(intmsg,	"\r\n[%s]\r\n",Storage.record_day[Storage.current_day].date);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));

		memset(intmsg, 0x00, 64);
		sprintf(intmsg,	"[%02d:%02d:%02d] current_day=%d \r\n",sh_info.houre,sh_info.minute,sh_info.second,
				Storage.current_day);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));

		memset(intmsg, 0x00, 64);
		sprintf(intmsg,	"current_index=%d write_address=0x%x total_entries=%d \r\n",Storage.record_day[Storage.current_day].current_index,
				write_address,Storage.record_day[Storage.current_day].total_entries);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));

		memset(intmsg, 0x00, 64);
		sprintf(intmsg,	"write data: pedo=%d alt=%d temp=%d hr=%d \r\n",record_data.pedo_counts,record_data.altitude,record_data.temperature,
				record_data.heart_rate);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));

		memset(&read_data, 0x00, sizeof(Data_Record));
		sys_data_read(write_address, &read_data, sizeof(Data_Record));
		memset(intmsg, 0x00, 64);
		sprintf(intmsg,	"read data: pedo=%d alt=%d temp=%d hr=%d \r\n",read_data.pedo_counts,read_data.altitude,read_data.temperature,
				read_data.heart_rate);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));

		memset(intmsg, 0x00, 64);
		sprintf(intmsg,
				"[%02d:%02d] %4d %3d %6d %3d %4d\r\n",
				sh_info.houre,
				sh_info.minute,
				sys_tick_buff[tmp_index],
				hr_info_buff[tmp_index],
				pedo_info_buff[tmp_index],
				temp_info_buff[tmp_index],
				alt_info_buff[tmp_index]);
		DRV_UART_Send(intmsg, strlen((const char *)intmsg));
#endif
	}

	//GPIOE->DOUT &= ~(1 << 6);
	//GPIOC->DOUT &= ~(1 << 6);

	if(bt_status == SBC2112_CONNECTED) {
		memset(intmsg, 0x00, 20);
		sprintf(intmsg,	"AT#SSD%02X%03X%03X%03X%03X\r\n", current_ticks & 0xFF, adc_data[1], adc_data[2], adc_data[3], adc_data[0]);
		//DRV_UART_Send(intmsg, strlen(intmsg));
		BT_UART_Write(UART1, intmsg, strlen((const char *)intmsg));
	}

	timer0_status = 1;

	return 0;
}

int DRV_TIMER1_Handler() {
	char intmsg[64];

	memset(intmsg, 0x00, 64);

	if(display_refresh_tick >= backlight_off_counts) {
		sprintf(intmsg, "\r\nturn off display....\r\n");
		DRV_UART_Send(intmsg, strlen(intmsg));
		display_refresh_tick = 0;
		display_on = 0;
		DRV_DISPLAY_SSD1306_Off();
		DRV_TIMER_Stop(TIMER_1);
		DRV_TIMER_Close(TIMER_1);
	} else {
		//sprintf(intmsg, "\r\nupdate display[%02d]......\r\n", display_refresh_tick);
		//DRV_UART_Send(intmsg, strlen(intmsg));
		if(bt_status == SBC2112_CONNECTED) display_refresh_tick = 0;
		sys_ui_update();
	}

	display_refresh_tick++;

	return 0;
}

int DRV_TIMER2_Handler() {

	return 0;
}

int DRV_TIMER3_Handler() {
	char intmsg[64];

	memset(intmsg, 0x00, 64);

	if(ble_connection_tick >= backlight_off_counts) {
		sprintf(intmsg, "\r\nTurn off BLE....\r\n");
		DRV_UART_Send(intmsg, strlen(intmsg));
		bt_status = SBC2112_DISCONNECT;
		sys_delay(40000);
		DRV_BT_SBC2112_Stop();
		DRV_TIMER_Stop(TIMER_3);
		DRV_TIMER_Close(TIMER_3);
	} else {
		if(bt_status == SBC2112_CONNECTED && sensorhub_state == STATE_BLE) {
			ble_connection_tick = 0;
		}
		sprintf(intmsg, "\r\nBLE Time ==> %3d\r\n", ble_connection_tick);
		DRV_UART_Send(intmsg, strlen(intmsg));
	}

	ble_connection_tick++;

	return 0;
}

void TMR0_IRQHandler(void) {
	TIMER0->ISR = 3;

	//GPIOC->DOUT |=  (1 << 6);
	DRV_TIMER0_Handler();
	//GPIOC->DOUT &= ~(1 << 6);
}

void TMR1_IRQHandler(void) {
	TIMER1->ISR = 3;
	DRV_TIMER1_Handler();
}

void TMR2_IRQHandler(void) {
	TIMER2->ISR = 3;
	DRV_TIMER2_Handler();
}

void TMR3_IRQHandler(void) {
	TIMER3->ISR = 3;
	DRV_TIMER3_Handler();
}
