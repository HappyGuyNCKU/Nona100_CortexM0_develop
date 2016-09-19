#include <math.h>

#include "algo_spo2hr.h"

#define XV_LEN		3
#define YV_LEN		3
#define HR_BUFF_LEN	4
#define WINDOW_LEN	8

#define SPO2_FS_HZ	50
#define SEC_PER_MIN	60
#define TICK_PER_MIN	(SPO2_FS_HZ * SEC_PER_MIN)

#define RESET_DURATION	200

static int last_value = 0;
static int last_direction = 0;
static int localmax = -4096;	// 13bit lower
static int localmin = 4095;
static int last_peaktime = 0;
static int pre_hr = 0;

static int xv[XV_LEN];
static int yv[XV_LEN];
static int threshold_buffer[HR_BUFF_LEN];
static int heartrate_buffer[HR_BUFF_LEN];
static int move_buffer[WINDOW_LEN];

extern volatile unsigned int current_ticks;

int spo2HRDect_open() {
	int i = 0;

	for(i = 0; i < XV_LEN; i++) {
		xv[i] = 0;
		yv[i] = 0;
	}

	for(i = 0; i < HR_BUFF_LEN; i++) {
		threshold_buffer[i] = 0;
		heartrate_buffer[i] = 0;
	}

	for(i = 0; i < WINDOW_LEN; i++)	move_buffer[i] = 0;

	pre_hr = 75;

	return 0;
}

int spo2HRDect_process(int raw_value) {
	int i = 0;
	int MA = 0;

	xv[0] = xv[1];                      //bandpass ; 1-st order ; 0.5~3Hz
	xv[1] = xv[2];
	xv[2] = ((raw_value>>3)+(raw_value>>5)+(raw_value>>6));// 1/6.9~=0.145==0.125+0.015625+0.0078125
			//raw_value/GAIN;

	yv[0] = yv[1];
	yv[1] = yv[2];
	yv[2] = (xv[2] - xv[0])-( (yv[0]>>1) + (yv[0]>>2) - (yv[0]>>5)) + (  yv[1]+ (yv[1]>>1)+(yv[1]>>2)-(yv[1]>>4)  );
	//-y[0]*(0.5+0.25-0.03125)+yv[1]*(1+0.5+0.25-0.0625)
	//+(int)( -0.7265425280 * yv[0]) + (int)(  1.7059650540 * yv[1]);

	//move_buffer[counter%8]=yv[2];		//movingWindowIntegeal ; length:8

	move_buffer[0] = move_buffer[1];
	move_buffer[1] = move_buffer[2];
	move_buffer[2] = move_buffer[3];
	move_buffer[3] = move_buffer[4];
	move_buffer[4] = move_buffer[5];
	move_buffer[5] = move_buffer[6];
	move_buffer[6] = move_buffer[7];
	move_buffer[7] = yv[2];

	for(i = 0;i < WINDOW_LEN; i++)	MA += move_buffer[i];

	MA = (MA+4) >> 3;

	return MA;
}

int spo2HRDect_detection(int filter_value) {
	int i = 0;
	int diff = 0;
	int duration = 0;
	int heartrate = 0;
	int tmp_hr_max = 0;
	int tmp_hr_min = 0;
	int threshold = 0;
	int peakdetect = 0;
	int direction = (filter_value > last_value ? 1 : (filter_value < last_value ? -1 : 0));

	//calculate dynamic threshold
	for(i = 0; i < HR_BUFF_LEN; i++)	threshold += threshold_buffer[i];
	threshold = (threshold+2) >> 2;
	//threshold*=0.5;

	if(direction == -last_direction && direction < 0 && last_value > localmax)	localmax = filter_value;
	if(direction == -last_direction && direction > 0 && last_value < localmin)	localmin = filter_value;

	diff = localmax - localmin;

	if ( diff > threshold * 0.5){//&& ( current_samplingtime-last_peaktime > 300  ||  ( last_peaktime > current_samplingtime &&  (3600000+current_samplingtime) > last_peaktime ) ) ){
		//detect one peak
		//calculate heartrate

		if (current_ticks > last_peaktime)
			duration = current_ticks - last_peaktime;
		else
			duration = 0xFFFFFFFF + current_ticks - last_peaktime;

		heartrate_buffer[0] = heartrate_buffer[1];
		heartrate_buffer[1] = heartrate_buffer[2];
		heartrate_buffer[2] = heartrate_buffer[3];
		heartrate_buffer[3] = duration;

		//update dynamic threshold
		threshold_buffer[0] = threshold_buffer[1];
		threshold_buffer[1] = threshold_buffer[2];
		threshold_buffer[2] = threshold_buffer[3];
		threshold_buffer[3] = diff;

		//update memory
		localmax = -4096;
		localmin = 4095;
		last_peaktime = current_ticks;
		peakdetect=1;
	}

	if(current_ticks - last_peaktime > RESET_DURATION){
		//update dynamic threshold
		threshold_buffer[0] = threshold_buffer[1];
		threshold_buffer[1] = threshold_buffer[2];
		threshold_buffer[2] = threshold_buffer[3];
		threshold_buffer[3] = diff;
	}

	//update memory
	last_value = filter_value;
    if(direction != 0) last_direction = direction;

	for(i = 0; i < HR_BUFF_LEN; i++)	heartrate += heartrate_buffer[i];
	heartrate = (heartrate + 2) >> 2;//avg time interval

	heartrate = (TICK_PER_MIN) / heartrate;

	tmp_hr_max = pre_hr + 15;
	tmp_hr_min = pre_hr - 15;

	if(heartrate > tmp_hr_min && heartrate < tmp_hr_max) {
		pre_hr = heartrate;
	} else {
		heartrate = pre_hr;
	}

	return heartrate;
}

int spo2HRDect_close() {

	return 0;
}
