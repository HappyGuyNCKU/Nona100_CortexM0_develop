#include "sys_util.h"

unsigned char dec2hex[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'A', 'B',
	'C', 'D', 'E', 'F'
};

inline void NOP(void) {
	__asm__ volatile("nop");
}

inline void sys_delay(unsigned int count) {
	volatile unsigned int i = count;

	// 7518 ==> 10ms delay
	//  749 ==>  1ms delay
	while (i--) {
		__asm__ volatile("nop");
	}
}

unsigned int hex_to_dec(unsigned char data) {
	unsigned char value = 0x00;

	switch(data) {
		case '0':
			value = 0;
			break;
		case '1':
			value = 1;
			break;
		case '2':
			value = 2;
			break;
		case '3':
			value = 3;
			break;
		case '4':
			value = 4;
			break;
		case '5':
			value = 5;
			break;
		case '6':
			value = 6;
			break;
		case '7':
			value = 7;
			break;
		case '8':
			value = 8;
			break;
		case '9':
			value = 9;
			break;
		case 'A':
		case 'a':
			value = 10;
			break;
		case 'B':
		case 'b':
			value = 11;
			break;
		case 'C':
		case 'c':
			value = 12;
			break;
		case 'D':
		case 'd':
			value = 13;
			break;
		case 'E':
		case 'e':
			value = 14;
			break;
		case 'F':
		case 'f':
			value = 15;
			break;
		default:
			value = 0;
			break;
	}

	return value;
}

unsigned char dec_to_hex(unsigned int value) {
	unsigned char code = '?';

	if(value <= 16)	code = dec2hex[value];

	return code;
}
