#include "nano1xx.h"
#include "nano1xx_uart.h"

#include "drv_uart.h"

UART_TypeDef *con_port = UART0;
UART_TypeDef *ble_port = UART1;

int DRV_UART_Init() {
	STR_UART_T sParam;

	// UART Setting
	sParam.u32BaudRate 			= 115200;
	sParam.u32cDataBits 		= DRVUART_DATABITS_8;
	sParam.u32cStopBits 		= DRVUART_STOPBITS_1;
	sParam.u32cParity 			= DRVUART_PARITY_NONE;
	sParam.u32cRxTriggerLevel	= DRVUART_FIFO_1BYTES;
	sParam.u8EnableDiv16		= DISABLE;

	// Set UART Configuration
	MFP_UART0_TO_PORTA();
	if(UART_Init(con_port, &sParam) != E_SUCCESS) {
		return -1;
	}

	// UART Setting
    sParam.u32BaudRate 			= 115200;
    sParam.u32cDataBits 		= DRVUART_DATABITS_8;
    sParam.u32cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u32cParity 			= DRVUART_PARITY_NONE;
    sParam.u32cRxTriggerLevel	= DRVUART_FIFO_1BYTES;
	sParam.u8EnableDiv16		= DISABLE;

	MFP_UART1_TO_PORTB();
	if(UART_Init(ble_port, &sParam) != E_SUCCESS) {
		return -2;
	}

	return 0;
}

int DRV_UART_Start() {
	UART_EnableInt(con_port, (DRVUART_RLSINT | DRVUART_THREINT | DRVUART_RDAINT));

	return 0;
}

void DRV_UART_Handler(unsigned int u32IntStatus) {

}

int DRV_UART_Send(unsigned char *data, unsigned int datalen) {
	int ret = 0;

	ret = UART_Write(con_port, data, datalen);

	return ret;
}

int DRV_UART_Recv(unsigned char *data, unsigned int datalen) {
	int ret = 0;

	ret = UART_Read(con_port, data, datalen);

	return ret;
}

int DRV_UART_Stop() {
	UART_DisableInt(con_port, DRVUART_RLSINT | DRVUART_THREINT | DRVUART_RDAINT);

	return 0;
}

void UART0_IRQHandler(void) {
	//unsigned int u32uart0IntStatus;

	//u32uart0IntStatus = UART0->ISR;

	//DRV_UART_Handler(u32uart0IntStatus);
}

void UART1_IRQHandler(void) {
	//unsigned int u32uart0IntStatus;

	//u32uart0IntStatus = UART1->ISR;

	//DRV_UART_Handler(u32uart0IntStatus);
}
