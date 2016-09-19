#include <stdio.h>

#include "nano1xx.h"

#include "drv_timer.h"
#include "drv_bt_SBC2112.h"
#include "drv_display_SSD1306.h"

#include "sys_ui.h"
#include "sys_info.h"

#define FONT_WIDTH		16
#define FONT_HEIGHT		32
#define SYMBOL_HEIGHT	32

int symbol_width = 0;
int symbol_height = 0;
int symbol_len = 0;
unsigned char state_msg[8];
unsigned char font_buffer[(FONT_WIDTH * FONT_HEIGHT) >> 3];

const unsigned char ITRI_ICON[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x0F, 0x0F, 0x06, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x60, 0xF0, 0xB8, 0xEC, 0xFC, 0xFE, 0xBE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char PEDO_ICON[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xF8, 0xBC, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x7E, 0x1E, 0x85, 0xFF, 0xFF, 0xFF, 0x1F, 0x7F, 0xB8, 0xE0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xFF, 0x7F, 0x0F, 0x0B, 0x3E, 0xFC, 0xF0, 0xE1, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1F, 0x0F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x1E, 0x1C, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char TEMP_ICON[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x18, 0x08, 0x08, 0x18, 0xF0, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xF8, 0xF8, 0x00, 0xFF, 0x00, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x30, 0xD8, 0xEF, 0xF0, 0xFF, 0xFF, 0xE0, 0xEF, 0x90, 0x62, 0xC2, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0C, 0x13, 0x37, 0x2F, 0x2F, 0x27, 0x2F, 0x17, 0x1B, 0x0C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char HR_ICON[] = {
	0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF4, 0xF8, 0xF0, 0xF0, 0xC0, 0xC0, 0xF0, 0xF0, 0xF8, 0xF4, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x00,
	0x0F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x0F,
	0x00, 0x00, 0x01, 0x03, 0x06, 0x0F, 0x1D, 0x37, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x37, 0x1D, 0x0F, 0x06, 0x03, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x0F, 0x1D, 0x37, 0x37, 0x1D, 0x0F, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char ALT_ICON[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x50, 0x1C, 0x03, 0x00, 0x00, 0x03, 0x1C, 0xE0, 0x80, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x1E, 0x83, 0xC1, 0x06, 0xC1, 0xF0, 0xA8, 0xE0, 0xFE, 0xFE, 0xFE, 0xF8, 0xE0, 0x03, 0xE0, 0xC3, 0x9E, 0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xE0, 0xFC, 0xFE, 0xFE, 0xDD, 0xBE, 0xF6, 0xF7, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x6E, 0x7E, 0x78, 0x70, 0x40,
};

const unsigned char BLE_ICON[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE, 0xF8, 0x70, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0E, 0x1C, 0x38, 0x30, 0xE0, 0xFF, 0xFF, 0xE0, 0x70, 0x29, 0x1F, 0x0E, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x38, 0x1C, 0x06, 0x06, 0x07, 0xDF, 0xFF, 0x07, 0x0E, 0x94, 0xF8, 0x70, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x1F, 0x0E, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char BLE_CONN_ICON[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0xC0, 0x04, 0x0E, 0x1F, 0x7E, 0xFC, 0xF0, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0xC0, 0x60, 0xC0, 0x80, 0x00, 0x00, 0x08, 0x1C, 0xFE, 0xFC, 0xF0, 0x00, 0x01, 0x07, 0x3F, 0xFF, 0xFC, 0xE0, 0x00, 0x01, 0x0F, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x03, 0x06, 0x03, 0x01, 0x00, 0x00, 0x10, 0x38, 0x7F, 0x3F, 0x0F, 0x00, 0x80, 0xE0, 0xFC, 0xFF, 0x3F, 0x07, 0x00, 0x80, 0xF0, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x07, 0x03, 0x20, 0x70, 0xF8, 0x7E, 0x3F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char BLE_NOCONN_ICON[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

extern Sensor_Hub_Info sh_info;
extern volatile int bt_status;
extern volatile unsigned int sensorhub_state;
extern volatile unsigned int battery_capacity;
extern Data_Storage Storage;

extern unsigned int  pedo_info_val;
extern unsigned char hr_info_val;
extern 		   short temp_info_val;
extern         short alte_info_val;

static int sys_ui_drawBattery(int percentage);
static int sys_ui_drawCapacity();
static int sys_ui_drawText(unsigned int offset, unsigned char *text);
static int sys_ui_drawChar(unsigned char char_val, unsigned char *buffer);

int sys_ui_logo() {
	symbol_width = 32;
	symbol_height = SYMBOL_HEIGHT;
	symbol_len = (symbol_width * symbol_height) >> 3;

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);
	DRV_DISPLAY_SSD1306_Write(ITRI_ICON, symbol_len, 0, symbol_width - 1, 2, 5);

	return 0;
}

int sys_ui_time() {

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);

	return 0;
}

int sys_ui_date() {

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);

	return 0;
}

int sys_ui_pedo() {
	symbol_width = 30;
	symbol_height = SYMBOL_HEIGHT;
	symbol_len = (symbol_width * symbol_height) >> 3;

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);
	DRV_DISPLAY_SSD1306_Write(PEDO_ICON, symbol_len, 0, symbol_width - 1, 2, 5);

	return 0;
}

int sys_ui_temp() {
	symbol_width = 30;
	symbol_height = SYMBOL_HEIGHT;
	symbol_len = (symbol_width * symbol_height) >> 3;

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);
	DRV_DISPLAY_SSD1306_Write(TEMP_ICON, symbol_len, 0, symbol_width - 1, 2, 5);

	return 0;
}

int sys_ui_hr() {
	symbol_width = 32;
	symbol_height = SYMBOL_HEIGHT;
	symbol_len = (symbol_width * symbol_height) >> 3;

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);
	DRV_DISPLAY_SSD1306_Write(HR_ICON, symbol_len, 0, symbol_width - 1, 2, 5);

	return 0;
}

int sys_ui_alt() {
	symbol_width = 30;
	symbol_height = SYMBOL_HEIGHT;
	symbol_len = (symbol_width * symbol_height) >> 3;

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);
	DRV_DISPLAY_SSD1306_Write(ALT_ICON, symbol_len, 0, symbol_width - 1, 2, 5);

	return 0;
}

int sys_ui_bleconn() {
	int offset = 0;
	static int state = 0;

	symbol_width = 30;
	offset = symbol_width;
	symbol_height = SYMBOL_HEIGHT;
	symbol_len = (symbol_width * symbol_height) >> 3;

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);
	DRV_DISPLAY_SSD1306_Write(BLE_ICON, symbol_len, offset, offset + symbol_width - 1, 2, 5);

	if(bt_status == SBC2112_CONNECTED) {
		offset = offset + symbol_width - 1;
		DRV_DISPLAY_SSD1306_Write(BLE_CONN_ICON, symbol_len, offset, offset + symbol_width - 1, 2, 5);
	} else {
		offset = offset + symbol_width - 1;
		DRV_DISPLAY_SSD1306_Write(BLE_NOCONN_ICON, symbol_len, offset, offset + symbol_width - 1, 2, 5);

		switch(state) {
			case 0:
				sys_ui_drawChar('\\', font_buffer);
				DRV_DISPLAY_SSD1306_Write(font_buffer, (32 * 16 / 8), offset, offset + FONT_WIDTH - 1, 2, 5);
				break;

			case 1:
				sys_ui_drawChar('|', font_buffer);
				DRV_DISPLAY_SSD1306_Write(font_buffer, (32 * 16 / 8), offset, offset + FONT_WIDTH - 1, 2, 5);
				break;

			case 2:
				sys_ui_drawChar('/', font_buffer);
				DRV_DISPLAY_SSD1306_Write(font_buffer, (32 * 16 / 8), offset, offset + FONT_WIDTH - 1, 2, 5);
				break;

			case 3:
				sys_ui_drawChar('-', font_buffer);
				DRV_DISPLAY_SSD1306_Write(font_buffer, (32 * 16 / 8), offset, offset + FONT_WIDTH - 1, 2, 5);
				break;
		}
		state++;
		state = state & 0x03;
	}

	return 0;
}

int sys_ui_update() {
	int draw = 1;
	unsigned int offset = 0;
	memset(state_msg, 0x00, 8);

	sys_ui_drawCapacity();
	sys_ui_drawBattery(battery_capacity);

	draw = 1;
	switch(sensorhub_state) {
		case STATE_LOGO:
			sys_ui_logo();
			offset = 32;
			sprintf(state_msg, "ITRI");
			break;

		case STATE_DATE:
			//sys_ui_date();
			offset = 0;
			sprintf(state_msg, "%04d%02d%02d", sh_info.year, sh_info.month, sh_info.day);
			break;

		case STATE_TIME:
			//sys_ui_time();
			offset = 0;
			sprintf(state_msg, "%02d:%02d:%02d", sh_info.houre, sh_info.minute, sh_info.second);
			break;

		case STATE_PEDO:
			//sys_ui_pedo();
			offset = 32;
			sprintf(state_msg, "%5d", pedo_info_val);
			break;

		case STATE_TEMP:
			//sys_ui_temp();
			offset = 32;
			if(temp_info_val >= 650) {
				sprintf(state_msg, " 65.0");
			} else if(temp_info_val < 0) {
				sprintf(state_msg, " 0.0");
			} else {
				sprintf(state_msg, "%2d.%1d", temp_info_val / 10, (temp_info_val % 10));
			}
			state_msg[strlen(state_msg)] = '.' | 'C';
			break;

		case STATE_HR:
			//sys_ui_hr();
			offset = 32;
			if(hr_info_val >= 200) {
				sprintf(state_msg, "200");
			} else if(hr_info_val <= 0) {
				sprintf(state_msg, " 0");
			} else {
				sprintf(state_msg, "%3d", hr_info_val);
			}
			break;

		case STATE_ALT:
			//sys_ui_alt();
			offset = 32;
			if(alte_info_val > 10000) {
				sprintf(state_msg, " 9999M");
			} else if(alte_info_val < -10000) {
				sprintf(state_msg, "-9999M");
			} else {
				sprintf(state_msg, " %4dM", alte_info_val);
			}
			break;

		case STATE_BLE_MAC:
			offset = 0;
			sprintf(state_msg, "%s", sh_info.ble_mac + 9);
			break;

		case STATE_BLE:
			sys_ui_bleconn();
			draw = 0;
			break;

		default:
			draw = 0;
			break;
	};

	if(draw)	sys_ui_drawText(offset, state_msg);

	return 0;
}

static int sys_ui_drawBattery(int percentage) {
	int i = 0;
	int empty = 0;
	unsigned char battery[24];

	empty = ((100 - percentage) / 5) + 3;
	battery[0]  = 0x3C;
	battery[1]  = 0x3C;
	battery[2]  = 0xFF;
	for(i = 3; i < 24; i++) battery[i] = i < empty ? 0x81 : 0xFF;

	DRV_DISPLAY_SSD1306_Write(battery, 24, 104, 127, 0, 0);

	return 0;
}

static int sys_ui_drawCapacity() {
	int i = 0;
	int cap = 0;
	int record_counts = 0;
	unsigned char capacity[82];

	memset(capacity, 0x81, 82);
	capacity[0]  = 0xFF;
	capacity[81]  = 0xFF;
	for (i = 0; i < RECORD_DAY_MAX; i++) {
		record_counts = record_counts + Storage.record_day[i].total_entries;
	}
	cap = (int)((record_counts * 80) / (RECORD_DAY_MAX * MINUTE_PER_DAY));
	for(i = 1; i < cap; i++) capacity[i] = (i & 0x01) ? 0xFF : 0x81;

	DRV_DISPLAY_SSD1306_Write(capacity, 82, 0, 81, 0, 0);

	return 0;
}

static int sys_ui_drawText(unsigned int offset, unsigned char *text) {
	int ret = 0;
	int len = 0;
	unsigned char *ptr = NULL;

	len = (FONT_WIDTH * FONT_HEIGHT) >> 3;

	ptr = text;
	while(1) {
		if(ptr[0] == 0x00) break;
		ret = sys_ui_drawChar(ptr[0], font_buffer);
		if(ret == 0)	DRV_DISPLAY_SSD1306_Write(font_buffer, len, offset, offset + FONT_WIDTH - 1, 2, 5);
		ptr++;
		offset += FONT_WIDTH;
		if(offset >= W_DOTS) break;
	}

	return 0;
}

static int sys_ui_drawChar(unsigned char char_val, unsigned char *buffer) {
	int i = 0;
	int end = 0;
	int ret = 0;
	int width = 0;
	unsigned char *ptr = NULL;

	end = FONT_WIDTH - 3;
	i = (FONT_WIDTH * FONT_HEIGHT) >> 3;
	memset(font_buffer, 0x00, i);

	switch(char_val) {
		case '0':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0xC0;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case '1':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[end - 2] = 0xFF;
			ptr[end - 1] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[end - 2] = 0xFF;
			ptr[end - 1] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[end - 2] = 0xFF;
			ptr[end - 1] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[end - 2] = 0xFF;
			ptr[end - 1] = 0xFF;
			break;

		case '2':
			ptr = buffer + 0 * FONT_WIDTH;
			for(i = 1; i < end; i++) ptr[i] = 0x03;
			ptr[end + 0] = 0xFF;
			ptr[end + 1] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			for(i = 1; i < FONT_WIDTH - 2; i++) ptr[i] = 0x80;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0x01;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0xC0;
			break;

		case '3':
			ptr = buffer + 0 * FONT_WIDTH;
			for(i = 1; i < end; i++) ptr[i] = 0x03;
			ptr[end + 0] = 0xFF;
			ptr[end + 1] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			for(i = 1; i < FONT_WIDTH - 2; i++) ptr[i] = 0x80;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			for(i = 1; i < FONT_WIDTH - 2; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			for(i = 1; i < end; i++) ptr[i] = 0xC0;
			ptr[end + 0] = 0xFF;
			ptr[end + 1] = 0xFF;
			break;

		case '4':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 2; i++) ptr[i] = 0x80;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 1; i < FONT_WIDTH - 2; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case '5':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0x80;

			ptr = buffer + 2 * FONT_WIDTH;
			for(i = 1; i < FONT_WIDTH - 1; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			for(i = 1; i < FONT_WIDTH - 1; i++) ptr[i] = 0xC0;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case '6':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0x80;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < FONT_WIDTH - 1; i++) ptr[i] = 0xC0;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case '7':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case '8':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x80;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0xC0;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case '9':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x80;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			for(i = 1; i < end; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			for(i = 1; i < end; i++) ptr[i] = 0xC0;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case 'A':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[5] = 0xFF;
			ptr[6] = 0xFF;
			ptr[7] = 0x03;
			ptr[8] = 0x03;
			ptr[9] = 0xFF;
			ptr[10] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[4] = 0xFF;
			ptr[5] = 0xFF;
			for(i = 6; i <= 9; i++) ptr[i] = 0x80;
			ptr[10] = 0xFF;
			ptr[11] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[3] = 0xFF;
			ptr[4] = 0xFF;
			for(i = 5; i <= 10; i++) ptr[i] = 0x01;
			ptr[11] = 0xFF;
			ptr[12] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[2] = 0xFF;
			ptr[3] = 0xFF;
			ptr[12] = 0xFF;
			ptr[13] = 0xFF;
			break;

		case 'B':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i <= 11; i++) ptr[i] = 0x03;
			ptr[12] = 0x0F;
			ptr[13] = 0x3C;
			ptr[14] = 0xF0;
			ptr[15] = 0xF0;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end + 1; i++) ptr[i] = 0x80;
			ptr[12] = 0xF0;
			ptr[13] = 0x3C;
			ptr[14] = 0x0F;
			ptr[15] = 0x0F;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end + 1; i++) ptr[i] = 0x01;
			ptr[12] = 0x0F;
			ptr[13] = 0x3C;
			ptr[14] = 0xF0;
			ptr[15] = 0xF0;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i <= 11; i++) ptr[i] = 0xC0;
			ptr[12] = 0xF0;
			ptr[13] = 0x3C;
			ptr[14] = 0x0F;
			ptr[15] = 0x0F;
			break;

		case 'C':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[3] = 0xFF;
			ptr[4] = 0xFF;
			for(i = 5; i < FONT_WIDTH - 1; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[3] = 0xFF;
			ptr[4] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[3] = 0xFF;
			ptr[4] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[3] = 0xFF;
			ptr[4] = 0xFF;
			for(i = 5; i < FONT_WIDTH - 1; i++) ptr[i] = 0xC0;
			break;

		case 'D':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i <= 11; i++) ptr[i] = 0x03;
			ptr[12] = 0x0F;
			ptr[13] = 0x3C;
			ptr[14] = 0xF0;
			ptr[15] = 0xF0;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[14] = 0xFF;
			ptr[15] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[14] = 0xFF;
			ptr[15] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i <= 11; i++) ptr[i] = 0xC0;
			ptr[12] = 0xF0;
			ptr[13] = 0x3C;
			ptr[14] = 0x0F;
			ptr[15] = 0x0F;
			break;

		case 'E':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x80;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x01;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0xC0;
			break;

		case 'F':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x80;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x01;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			break;

		case 'I':
			ptr = buffer + 0 * FONT_WIDTH;
			for(i = 4; i <= 11; i++) ptr[i] = 0x03;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			for(i = 4; i <= 11; i++) ptr[i] = 0xC0;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;
			break;

		case 'L':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end + 2; i++) ptr[i] = 0xC0;
			break;

		case 'M':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[3] = 0x0F;
			ptr[4] = 0x0F;
			ptr[5] = 0xF0;
			ptr[6] = 0xF0;
			ptr[9] = 0xF0;
			ptr[10] = 0xF0;
			ptr[11] = 0x0F;
			ptr[12] = 0x0F;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[5] = 0x0F;
			ptr[6] = 0x0F;
			ptr[7] = 0xF0;
			ptr[8] = 0xF0;
			ptr[9] = 0x0F;
			ptr[10] = 0x0F;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case 'R':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0x03;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end + 1; i++) ptr[i] = 0x80;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end + 1; i++) ptr[i] = 0x01;
			ptr[FONT_WIDTH - 5] = 0xFF;
			ptr[FONT_WIDTH - 4] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case 'T':
			ptr = buffer + 0 * FONT_WIDTH;
			for(i = 2; i < (FONT_WIDTH - 1); i++) ptr[i] = 0x03;
			ptr[1] = 0x0F;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0x0F;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			for(i = 6; i <= 9; i++) ptr[i] = 0xC0;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;
			break;

		case 'U':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[1] = 0xFF;
			ptr[2] = 0xFF;
			for(i = 3; i < end; i++) ptr[i] = 0xC0;
			ptr[FONT_WIDTH - 3] = 0xFF;
			ptr[FONT_WIDTH - 2] = 0xFF;
			break;

		case ('.' | 'C'):
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xF0;
			ptr[2] = 0xF0;
			ptr[3] = 0xF0;
			ptr[4] = 0xF0;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;
			for(i = 8; i < FONT_WIDTH - 1; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;
			for(i = 8; i < FONT_WIDTH - 1; i++) ptr[i] = 0xC0;
			break;

		case ('.' | 'F'):
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[1] = 0xF0;
			ptr[2] = 0xF0;
			ptr[3] = 0xF0;
			ptr[4] = 0xF0;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;
			for(i = 8; i < FONT_WIDTH - 1; i++) ptr[i] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			for(i = 8; i < end; i++) ptr[i] = 0x80;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			for(i = 8; i < end; i++) ptr[i] = 0x01;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[6] = 0xFF;
			ptr[7] = 0xFF;
			break;

		case ':':
			ptr = buffer + 1 * FONT_WIDTH;
			ptr[7]  = 0x0F;
			ptr[8]  = 0x0F;
			ptr[9]  = 0x0F;
			ptr[10] = 0x0F;
			ptr = buffer + 2 * FONT_WIDTH;
			ptr[7]  = 0xF0;
			ptr[8]  = 0xF0;
			ptr[9]  = 0xF0;
			ptr[10] = 0xF0;
			break;

		case '.':
			ptr = buffer + 3 * FONT_WIDTH;
			ptr[FONT_WIDTH - 7] = 0x06;
			ptr[FONT_WIDTH - 6] = 0x0F;
			ptr[FONT_WIDTH - 5] = 0x0F;
			ptr[FONT_WIDTH - 4] = 0x06;
			break;

		case '-':
			ptr = buffer + 1 * FONT_WIDTH;
			for(i = 4; i < end; i++) ptr[i] = 0x80;
			ptr = buffer + 2 * FONT_WIDTH;
			for(i = 4; i < end; i++) ptr[i] = 0x01;
			break;

		case '|':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[7] = 0xFF;
			ptr[8] = 0xFF;
			break;

		case '\\':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[0] = 0x03;
			ptr[1] = 0x0C;
			ptr[2] = 0x30;
			ptr[3] = 0xC0;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[4] = 0x03;
			ptr[5] = 0x0C;
			ptr[6] = 0x30;
			ptr[7] = 0xC0;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[8] = 0x03;
			ptr[9] = 0x0C;
			ptr[10] = 0x30;
			ptr[11] = 0xC0;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[12] = 0x03;
			ptr[13] = 0x0C;
			ptr[14] = 0x30;
			ptr[15] = 0xC0;
			break;

		case '/':
			ptr = buffer + 0 * FONT_WIDTH;
			ptr[12] = 0xC0;
			ptr[13] = 0x30;
			ptr[14] = 0x0C;
			ptr[15] = 0x03;

			ptr = buffer + 1 * FONT_WIDTH;
			ptr[8] = 0xC0;
			ptr[9] = 0x30;
			ptr[10] = 0x0C;
			ptr[11] = 0x03;

			ptr = buffer + 2 * FONT_WIDTH;
			ptr[4] = 0xC0;
			ptr[5] = 0x30;
			ptr[6] = 0x0C;
			ptr[7] = 0x03;

			ptr = buffer + 3 * FONT_WIDTH;
			ptr[0] = 0xC0;
			ptr[1] = 0x30;
			ptr[2] = 0x0C;
			ptr[3] = 0x03;
			break;

		case ' ':
			break;

		default:
			ret = -1;
			break;
	}

	return ret;
}

int sys_ui_clear() {
	DRV_DISPLAY_SSD1306_Clear();

	return 0;
}
