#ifndef DRV_UART_H
#define DRV_UART_H

int DRV_UART_Init();
int DRV_UART_Start();
void DRV_UART_Handler(unsigned int u32IntStatus);
int  DRV_UART_Send(unsigned char *data, unsigned int datalen);
int  DRV_UART_Recv(unsigned char *data, unsigned int datalen);
int DRV_UART_Stop();

#endif
