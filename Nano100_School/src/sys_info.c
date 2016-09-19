#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "drv_rtc.h"
#include "drv_uart.h"
#include "drv_timer.h"
#include "drv_flash_SST25VF032B.h"

#include "sys_info.h"

Sensor_Hub_Info sh_info;
Sensor_Info ecg_info;
Sensor_Info temp_info;
Sensor_Info acc_info;
Sensor_Info alt_info;

Sensor_Info *ecg_info_ptr = NULL;
Sensor_Info *temp_info_ptr = NULL;
Sensor_Info *acc_info_ptr = NULL;
Sensor_Info *alt_info_ptr = NULL;

Data_Storage Storage;
extern volatile unsigned char day_change;

unsigned short ecg_raw_buff[ECG_RAW_MAX];
unsigned short acc_all_raw_buff[ACC_RAW_MAX * 3];
unsigned short *acc_x_raw_buff = NULL;
unsigned short *acc_y_raw_buff = NULL;
unsigned short *acc_z_raw_buff = NULL;
unsigned short temp_raw_buff[TEMP_RAW_MAX];
unsigned short alt_raw_buff[ALT_RAW_MAX];

unsigned int   pedo_info_buff[PEDO_INFO_MAX];
unsigned char  hr_info_buff[HR_INFO_MAX];
		 short temp_info_buff[TEMP_INFO_MAX];
         short alt_info_buff[ALT_INFO_MAX];
unsigned short sys_tick_buff[SYS_TICK_MAX];
unsigned short current_sys_tick = 0;

unsigned int  pedo_info_val = 0;
unsigned char hr_info_val = 0;
		short temp_info_val = 0;
		short alte_info_val = 0;

unsigned char ecg_cmd[] =  "SET_ECG_ON\r\nSET_ECG_OFF\r\nSET_ECG_RATE MIN:100 MAX:500\r\nGET_ECG_RATE\r\nGET_ECG_STATUS\r\n";
unsigned char temp_cmd[] = "SET_TEMP_ON\r\nSET_TEMP_OFF\r\nSET_TEMP_RATE MIN:1 MAX:5\r\nGET_TEMP_RATE\r\nGET_TEMP_STATUS\r\n";
unsigned char acc_cmd[] =  "SET_ACC_ON\r\nSET_ACC_OFF\r\nSET_ACC_RATE MIN:10 MAX:100\r\nGET_ACC_RATE\r\nGET_ACC_STATUS\r\n";
unsigned char alt_cmd[] =  "SET_ALT_ON\r\nSET_ALT_OFF\r\nSET_ALT_RATE MIN:10 MAX:100\r\nGET_ALT_RATE\r\nGET_ALT_STATUS\r\n";

volatile unsigned int sensorhub_state = STATE_UNKNOWN;
volatile unsigned int battery_capacity = 0;

int sys_sensorhub_init() {
	memset(&sh_info, 0x00, sizeof(Sensor_Hub_Info));

	strcpy(sh_info.fw_version, FW_VERSION);
	strcpy(sh_info.sn_id, SN_ID);
	strcpy(sh_info.dev_id, DEV_ID);
	strcpy(sh_info.account, "USER0");
	strcpy(sh_info.session_id, "USER0");

	sh_info.year = 2014;
	sh_info.month = 12;
	sh_info.day = 24;
	sh_info.houre = 23;
	sh_info.minute = 58;
	sh_info.second = 40;
	sh_info.milisecond = 0;
	sh_info.timezone = 8;

	sh_info.battery_voltage_low = 2500;
	sh_info.battery_voltage_work = 3025;

	sensorhub_state = STATE_UNKNOWN;

	ecg_info_ptr = &ecg_info;
	temp_info_ptr = &temp_info;
	acc_info_ptr = &acc_info;
	alt_info_ptr = &alt_info;

	hr_info_val = 0;
	alte_info_val = 0;
	pedo_info_val = 0;
	temp_info_val = 0;

	battery_capacity = 0;
	current_sys_tick = 0;

	sys_storage_init();

	//sh_info.session_status = SESSION_UNKONW;

	return 0;
}

int sys_sensorhub_update() {

	// call DrvFMC for data update.
	DRV_RTC_Set();

	return 0;
}

int sys_sensorhub_dumpinfo() {
	char msg[64];

	memset(msg, 0x00, 64);
	sprintf(msg, "\r\nSensor Hub size: %d\r\n", sizeof(Sensor_Hub_Info));
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "FW:%s\r\n", sh_info.fw_version);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "SNID:%s\r\n", sh_info.sn_id);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "DEVID:%s\r\n", sh_info.dev_id);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "ACCOUNT:%s\r\n", sh_info.account);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "ACCOUNT:%s\r\n", sh_info.session_id);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "%04d-%02d-%02d %02d:%02d:%02d\r\n",
			sh_info.year,
			sh_info.month,
			sh_info.day,
			sh_info.houre,
			sh_info.minute,
			sh_info.second);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "TimeZone:%d\r\n",	sh_info.timezone);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "Voltage Low:%d\r\n",	sh_info.battery_voltage_low);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 64);
	sprintf(msg, "Voltage work:%d\r\n",	sh_info.battery_voltage_work);
	DRV_UART_Send(msg, strlen(msg));

	return 0;
}

int sys_sensorhub_read() {
	DRV_FLASH_SST25VF032B_ReadBytes(SENSORHUB_INFO_CURRENT,  (unsigned char *)&sh_info, sizeof(Sensor_Hub_Info));

	return 0;
}

int sys_sensorhub_write() {
	DRV_FLASH_SST25VF032B_DisableProtection();
	DRV_FLASH_SST25VF032B_4KErase(SENSORHUB_INFO_CURRENT);
	DRV_FLASH_SST25VF032B_WriteBytes(SENSORHUB_INFO_CURRENT, (unsigned char *)&sh_info, sizeof(Sensor_Hub_Info));
	DRV_FLASH_SST25VF032B_EnableProtection();

	return 0;
}

int sys_sensors_init()  {
	int i = 0;

	memset(&ecg_info,  0x00, sizeof(Sensor_Info));
	memset(&alt_info, 0x00, sizeof(Sensor_Info));
	memset(&temp_info, 0x00, sizeof(Sensor_Info));
	memset(&acc_info, 0x00, sizeof(Sensor_Info));

	strcpy(ecg_info.sensor_name, "ECG");
	ecg_info.rate_max = 500;
	ecg_info.rate_work = 200;
	ecg_info.rate_min = 100;
	ecg_info.sensor_status = SENSOR_ON;

	ecg_info.raw_data_ptr = &ecg_raw_buff;
	ecg_info.raw_data_max = ECG_RAW_MAX;
	ecg_info.raw_data_index = 0;

	ecg_info.info_data_ptr = &hr_info_buff;
	ecg_info.info_data_max = HR_INFO_MAX;
	ecg_info.info_data_index = 2;

	ecg_info.command_str = &ecg_cmd;

	strcpy(temp_info.sensor_name, "TEMP");
	temp_info.rate_max = 5;
	temp_info.rate_work = 2;
	temp_info.rate_min = 1;
	temp_info.sensor_status = SENSOR_ON;

	temp_info.raw_data_ptr = &temp_raw_buff;
	temp_info.raw_data_max = TEMP_RAW_MAX;
	temp_info.raw_data_index = 0;

	temp_info.info_data_ptr = &temp_info_buff;
	temp_info.info_data_max = TEMP_INFO_MAX;
	temp_info.info_data_index = 2;

	temp_info.command_str = &temp_cmd;

	strcpy(alt_info.sensor_name, "ALT");
	alt_info.rate_max = 5;
	alt_info.rate_min = 1;
	alt_info.sensor_status = SENSOR_ON;

	alt_info.raw_data_ptr = &alt_raw_buff;
	alt_info.raw_data_max = ALT_RAW_MAX;
	alt_info.raw_data_index = 0;

	alt_info.info_data_ptr = &alt_info_buff;
	alt_info.info_data_max = ALT_INFO_MAX;
	alt_info.info_data_index = 0;

	alt_info.command_str = &alt_cmd;

	strcpy(temp_info.sensor_name, "PEDO");
	acc_info.rate_max = 100;
	acc_info.rate_work = 50;
	acc_info.rate_min = 10;
	acc_info.sensor_status = SENSOR_ON;

	acc_info.raw_data_ptr = &acc_all_raw_buff;
	acc_info.raw_data_max = ACC_RAW_MAX * 3;
	acc_info.raw_data_index = 0;

	acc_info.info_data_ptr = &pedo_info_buff;
	acc_info.info_data_max = PEDO_INFO_MAX;
	acc_info.info_data_index = 0;

	acc_info.command_str = &acc_cmd;

	// buffer initialization

	memset(ecg_raw_buff,     0x00,     ECG_RAW_MAX * sizeof(unsigned short));
	memset(acc_all_raw_buff, 0x00, 3 * ACC_RAW_MAX * sizeof(unsigned short));
	memset(temp_raw_buff,    0x00,    TEMP_RAW_MAX * sizeof(unsigned short));
	memset(alt_raw_buff,     0x00,     ALT_RAW_MAX * sizeof(unsigned short));

	memset(pedo_info_buff, 0x00,  PEDO_INFO_MAX * sizeof(unsigned int));
	memset(hr_info_buff,   0x00,    HR_INFO_MAX * sizeof(unsigned char));
	memset(temp_info_buff, 0x00,  TEMP_INFO_MAX * sizeof(short));
	memset(alt_info_buff,  0x00,   ALT_INFO_MAX * sizeof(short));
	memset(sys_tick_buff,  0x00,   SYS_TICK_MAX * sizeof(unsigned short));

	acc_x_raw_buff = acc_all_raw_buff;
	acc_y_raw_buff = acc_x_raw_buff + ACC_RAW_MAX;
	acc_z_raw_buff = acc_y_raw_buff + ACC_RAW_MAX;

	// for dummy data generation
	for(i = 0; i < ECG_RAW_MAX; i++)  ecg_raw_buff[i] = i;
	for(i = 0; i < TEMP_RAW_MAX; i++) temp_raw_buff[i] = 2048 + i;
	for(i = 0; i < ALT_RAW_MAX; i++)  alt_raw_buff[i] = 2048 + i;

	return 0;
}

int sys_sensors_update(Sensor_Info *sensor_info)  {

	// call DrvFMC for data update.

	return 0;
}

int sys_sensors_read(Sensor_Info *sensor) {
	if(sensor == NULL)	return -1;

	return 0;
}

int sys_sensors_write(Sensor_Info *sensor) {
	if(sensor == NULL)	return -1;

	return 0;
}

int sys_storage_init() {
	int i = 0;

	memset(&Storage, 0x00, sizeof(Data_Storage));
	sprintf(Storage.record_day[Storage.current_day].date, "%04d-%02d-%02d", sh_info.year, sh_info.month, sh_info.day);

	for (i = 0; i < RECORD_DAY_MAX; i++) {
		Storage.record_day[i].begin_address = DATA_PAGE_BASE + DATA_PAGE_SIZE * i;
		Storage.record_day[i].end_address = Storage.record_day[i].begin_address + MINUTE_PER_DAY * sizeof(Data_Record);
	}

	return 0;
}

int sys_data_read(unsigned int address, unsigned char *buff, unsigned int length) {
	if(buff == NULL)	return -1;
	if(length <= 0)		return -1;

	DRV_FLASH_SST25VF032B_ReadBytes(address, buff, length);

	return 0;
}

int sys_data_write(unsigned int address, unsigned char *buff, unsigned int length) {
	int i = 0;

	if(buff == NULL)	return -1;
	if(length <= 0)		return -1;

	// check day change
	if(day_change == 1) {
		day_change = 0;
		// erase flash pages according to begin_address of record_day[]
		DRV_FLASH_SST25VF032B_DisableProtection();
		for(i = 0; i < DATA_PAGE_COUNT; i++) {
			DRV_FLASH_SST25VF032B_4KErase(Storage.record_day[Storage.current_day].begin_address + i * PAGE_SIZE);
			// should delay after every sector erase
			sys_delay(16930);
		}
		DRV_FLASH_SST25VF032B_EnableProtection();
	}

	// write data into flash
	DRV_FLASH_SST25VF032B_DisableProtection();
	DRV_FLASH_SST25VF032B_WriteBytes(address, buff, length);
	DRV_FLASH_SST25VF032B_EnableProtection();

	return 0;
}
