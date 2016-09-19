#ifndef SYS_UTIL_H
#define SYS_UTIL_H

inline void NOP(void);
inline void sys_delay(unsigned int count);
unsigned int hex_to_dec(unsigned char data);
unsigned char dec_to_hex(unsigned int value);

#endif
