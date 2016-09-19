#ifndef SYS_INFO_H
#define SYS_INFO_H

#define FW_VERSION  "SH_FW_20141101_2.0"
#define SN_ID		"00001"
#define DEV_ID		"SHPOC"

#define SENSOR_ON	1
#define SENSOR_OFF	0

#define RAW_ENABLE	1
#define RAW_DISABLE	0

#define ECG_RAW_MAX		256
#define ACC_RAW_MAX		128
#define TEMP_RAW_MAX	16
#define ALT_RAW_MAX		16

#define RECORD_DAY_MAX          2
#define DATA_INDEX_TOTAL		32
#define PAGE_SIZE				0x1000	// 4096 bytes
#define INFO_PAGE_BASE			0x000000

#define SENSORHUB_INFO_DEFAULT	 INFO_PAGE_BASE						// default sensor hub info and setting
#define SENSORHUB_INFO_CURRENT  (INFO_PAGE_BASE + (PAGE_SIZE * 1))	// current sensor hub info and setting
#define SENSORS_INFO_BASE		(INFO_PAGE_BASE + (PAGE_SIZE * 2))	// current sensors info and setting
#define DATA_INDEX_BASE			(INFO_PAGE_BASE + (PAGE_SIZE * 3))	// data storage index
#define DATA_PAGE_BASE			(INFO_PAGE_BASE + (PAGE_SIZE * 4))	// data storage area
#define DATA_PAGE_SIZE			(((24 * 60 * sizeof(Data_Record) + (PAGE_SIZE - 1)) / PAGE_SIZE) * PAGE_SIZE)
#define DATA_PAGE_COUNT			((24 * 60 * sizeof(Data_Record) + (PAGE_SIZE - 1)) / PAGE_SIZE)

#define SYS_TICK_MAX	128
#define HR_INFO_MAX		SYS_TICK_MAX
#define ALT_INFO_MAX	SYS_TICK_MAX
#define PEDO_INFO_MAX	SYS_TICK_MAX
#define TEMP_INFO_MAX	SYS_TICK_MAX

typedef struct {
	unsigned char fw_version[20];
	unsigned char ble_mac[20];
	unsigned char sn_id[8];
	unsigned char dev_id[8];
	unsigned char account[8];
	unsigned char session_id[8];

	// for sensor hub date, time setting
	unsigned short year;
	unsigned char  month;
	unsigned char  day;

	char  timezone;
	char  reserved[3];

	unsigned char  houre;
	unsigned char  minute;
	unsigned char  second;
	unsigned char  milisecond;	// only two digit

	// voltage unit : mili voltage, mV
	unsigned short  battery_voltage_low;
	unsigned short  battery_voltage_work;
} Sensor_Hub_Info;

typedef struct {
	unsigned char sensor_name[8];
	unsigned int rate_max;	// unit is Hz
	unsigned int rate_work; // unit is Hz
	unsigned int rate_min;	// unit is Hz
	unsigned int sensor_status;

	// for sensor information
	void *info_data_ptr;
	unsigned int info_data_max;
	unsigned int info_data_index;

	// for sensor raw data
	void *raw_data_ptr;
	unsigned int raw_data_max;
	unsigned int raw_data_index;

	unsigned int raw_enabled;

	void *command_str;
} Sensor_Info;

typedef enum {
	STATE_UNKNOWN = -1,
	STATE_LOGO = 0,
	STATE_DATE,
	STATE_TIME,
	STATE_PEDO,
	STATE_TEMP,
	STATE_HR,
	STATE_ALT,
	STATE_BLE_MAC,
	STATE_BLE,
	STATE_LAST
} SensorHub_State;

// For Save data into flash.
// Data_info for date is a key to arrange the real storage block assignment.
typedef struct {
	unsigned char date[12];
	unsigned int begin_address;
	unsigned int end_address;
	unsigned int current_index;
	unsigned int total_entries;
} Data_Info;

// Data_Record is 1 minute 1 entry.
typedef struct {
	unsigned int   pedo_counts;
	unsigned short altitude;
			 short temperature;
	unsigned char  heart_rate;
	unsigned char  reserved[3];
} Data_Record;

typedef struct {
	Data_Info     record_day[RECORD_DAY_MAX];
	unsigned char current_day;
} Data_Storage;

int sys_sensorhub_init();							// init sensor hub info
int sys_sensorhub_update();							// update sensor hub info into flash
int sys_sensorhub_dumpinfo();
int sys_sensorhub_read();
int sys_sensorhub_write();

int sys_sensors_init();		// init sensors info
int sys_sensors_update(Sensor_Info *sensor_info);	// update sensors info into flash
int sys_sensors_read(Sensor_Info *sensor);
int sys_sensors_write(Sensor_Info *sensor);

int sys_storage_init();

int sys_data_read(unsigned int address, unsigned char *buff, unsigned int length);
int sys_data_write(unsigned int address, unsigned char *buff, unsigned int length);

#endif
