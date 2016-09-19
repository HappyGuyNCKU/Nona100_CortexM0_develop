#include <stdlib.h>

#include "algo_pedo_r4.h"

#define WINDOW1_LEN 5
#define WINDOW2_LEN 5
#define WINDOW3_LEN 5
#define THRESHOLD_LEN 4
#define THRESHOLD_LEN2 200

#define RESET_DURATION	200
#define time_threshold 30//0.6sec

static int last_value  = 0;
static int last_direction = 0;
static int localmax = 4095;
static int localmin = -4096;
static int last_peaktime = 0;
static int step_num = 0;
static int time_offset=0;//after counting first 4sec data = 0 - 200 data count, start pedo

static int accel_x[WINDOW1_LEN];
static int accel_y[WINDOW1_LEN];
static int accel_z[WINDOW1_LEN];
static int accel_xx[WINDOW2_LEN];
static int accel_yy[WINDOW2_LEN];
static int accel_zz[WINDOW2_LEN];
static int accel_avg[WINDOW3_LEN];
static int threshold_buffer[THRESHOLD_LEN];
static int threshold_buffer2[THRESHOLD_LEN2];

extern volatile unsigned int current_ticks;

int pedoDect_R4_open() {
	int i = 0;

	for(i = 0; i < WINDOW1_LEN;i++){
		accel_x[i]=0;
		accel_y[i]=0;
		accel_z[i]=0;
	}

	for(i = 0; i < WINDOW2_LEN;i++){
		accel_xx[i]=0;
		accel_yy[i]=0;
		accel_zz[i]=0;
	}

	for(i = 0; i < WINDOW3_LEN;i++) accel_avg[i]=0;
	for(i = 0; i < THRESHOLD_LEN;i++) threshold_buffer[i]=0;
	for( i = 0; i < THRESHOLD_LEN2;i++) threshold_buffer2[i]=0;
}

int pedoDect_R4_process(int raw_valueX, int raw_valueY, int raw_valueZ) {
	int i=0;
	int MA_X=0,MA_Y=0,MA_Z=0;
	int MA_XX=0,MA_YY=0,MA_ZZ=0;
	int avg=0;//filter_value

	//x,y,z go through 1-st time movingWindowIntegral with length 8 int
	for( i=0;i<WINDOW1_LEN-2;i++){
		accel_x[i]=accel_x[i+1];
		accel_y[i]=accel_y[i+1];
		accel_z[i]=accel_z[i+1];
	}

	if(time_offset<200){
		raw_valueX=0;
		raw_valueY=0;
		raw_valueZ=0;
	}


	accel_x[WINDOW1_LEN-1]=raw_valueX;
	accel_y[WINDOW1_LEN-1]=raw_valueY;
	accel_z[WINDOW1_LEN-1]=raw_valueZ;

	for( i=0;i<WINDOW1_LEN;i++){
		MA_X+=accel_x[i];
		MA_Y+=accel_y[i];
		MA_Z+=accel_z[i];
	}

	if ( MA_X > 0)
		MA_X=(MA_X+WINDOW1_LEN>>1)/WINDOW1_LEN;
	else
		MA_X=(MA_X-WINDOW1_LEN>>1)/WINDOW1_LEN;

	if ( MA_Y > 0)
		MA_Y=(MA_Y+WINDOW1_LEN>>1)/WINDOW1_LEN;
	else
		MA_Y=(MA_Y-WINDOW1_LEN>>1)/WINDOW1_LEN;

	if ( MA_Z > 0)
		MA_Z=(MA_Z+WINDOW1_LEN>>1)/WINDOW1_LEN;
	else
		MA_Z=(MA_Z-WINDOW1_LEN>>1)/WINDOW1_LEN;

	//x,y,z go through 2-nd time movingWindowIntegral with length 8 int
	for( i=0;i<WINDOW2_LEN-2;i++){
		accel_xx[i]=accel_xx[i+1];
		accel_yy[i]=accel_yy[i+1];
		accel_zz[i]=accel_zz[i+1];
	}

	accel_xx[WINDOW2_LEN-1]=MA_X;
	accel_yy[WINDOW2_LEN-1]=MA_Y;
	accel_zz[WINDOW2_LEN-1]=MA_Z;

	for( i=0;i<WINDOW2_LEN;i++){
		MA_XX+=accel_xx[i];
		MA_YY+=accel_yy[i];
		MA_ZZ+=accel_zz[i];
	}

	if ( MA_XX > 0)
		MA_XX=(MA_XX+WINDOW2_LEN>>1)/WINDOW2_LEN;
	else
		MA_XX=(MA_XX-WINDOW2_LEN>>1)/WINDOW2_LEN;

	if ( MA_YY > 0)
		MA_YY=(MA_YY+WINDOW2_LEN>>1)/WINDOW2_LEN;
	else
		MA_YY=(MA_YY-WINDOW2_LEN>>1)/WINDOW2_LEN;

	if ( MA_ZZ > 0)
		MA_ZZ=(MA_ZZ+WINDOW2_LEN>>1)/WINDOW2_LEN;
	else
		MA_ZZ=(MA_ZZ-WINDOW2_LEN>>1)/WINDOW2_LEN;

	//combine x,y,z into 1 dimension and go through 3-rd time movingWindowIntegral with length 8 int
	for( i=0;i<WINDOW3_LEN-2;i++) accel_avg[i]=accel_avg[i+1];
	accel_avg[WINDOW3_LEN-1]=abs(MA_XX)+abs(MA_YY)+abs(MA_ZZ);
	for( i=0;i<WINDOW3_LEN;i++) avg+=accel_avg[i];
	avg=(avg+WINDOW3_LEN>>1)/WINDOW3_LEN;

	time_offset++;

	return avg;
}

int pedoDect_R4_detection(int filter_value){
	int i=0;
	int diff=0;
	int threshold=0;
	int threshold2=0;
	int direction = ( filter_value > last_value ? 1 : ( filter_value < last_value ? -1 : 0));

	for ( i=0; i<THRESHOLD_LEN; i++) threshold+=threshold_buffer[i];
	threshold=(threshold+THRESHOLD_LEN>>1)/THRESHOLD_LEN;

	for ( i=0; i<THRESHOLD_LEN2; i++) threshold2+=threshold_buffer2[i];
	threshold2=(threshold2+THRESHOLD_LEN2>>1)/THRESHOLD_LEN2;

	if ( direction == -last_direction && direction < 0 && last_value>localmax)	localmax = last_value;
	if ( direction == -last_direction && direction > 0 && last_value<localmin)	localmin = last_value;

	diff=localmax-localmin;
	if ( (diff> threshold*1.2) &&(diff> threshold2*5)&&(current_ticks - last_peaktime>time_threshold) ){
		step_num+=1;
		//update dynamic threshold
		for ( i=0; i<THRESHOLD_LEN-2; i++) threshold_buffer[i]=threshold_buffer[i+1];
		threshold_buffer[THRESHOLD_LEN-1]=diff;
		//update dynamic threshold2
		for ( i=0; i<THRESHOLD_LEN2-2; i++) threshold_buffer2[i]=threshold_buffer2[i+1];
		threshold_buffer2[THRESHOLD_LEN2-1]=diff;
		//update memory
		localmax = -4096;
		localmin =  4095;
		last_peaktime = current_ticks;
	}

	if ( current_ticks-last_peaktime>RESET_DURATION){
		//update dynamic threshold
		for ( i=0; i<THRESHOLD_LEN-2; i++) threshold_buffer[i]=threshold_buffer[i+1];
		threshold_buffer[THRESHOLD_LEN-1]=diff;
		//update dynamic threshold2
		for ( i=0; i<THRESHOLD_LEN2-2; i++) threshold_buffer2[i]=threshold_buffer2[i+1];
		threshold_buffer2[THRESHOLD_LEN2-1]=diff;

	}

	//update memory
	last_value=filter_value;
	if ( direction!=0) last_direction = direction;

	return step_num;
}

int pedoDect_R4_close() {

	return 0;
}
