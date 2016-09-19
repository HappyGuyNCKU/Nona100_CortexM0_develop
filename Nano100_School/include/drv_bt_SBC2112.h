#include "nano1xx.h"

#ifndef DRV_BT_SBC2112_H
#define DRV_BT_SBC2112_H

typedef enum{
	SBC2112_UNKNOWN = -1,
	SBC2112_READY = 0,
	SBC2112_CONNECTED,
	SBC2112_DISCONNECT,
} SBC2112_Status;

int DRV_BT_SBC2112_Init();
int DRV_BT_SBC2112_Wait_Connect();
int DRV_BT_SBC2112_Stop();
int DRV_BT_SBC2112_Send(unsigned char *databuff, int databufflen);
int DRV_BT_SBC2112_Recv(unsigned char *databuff, int databufflen);

int BT_UART_Read(UART_TypeDef  *tUART, unsigned char *pu8RxBuf, unsigned int u32ReadBytes);
int BT_UART_Write(UART_TypeDef *tUART, unsigned char *pu8TxBuf, unsigned int u32WriteBytes);

#endif
