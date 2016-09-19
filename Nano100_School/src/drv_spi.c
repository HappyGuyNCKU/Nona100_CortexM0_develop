#include <stdio.h>

#include "nano1xx.h"
#include "nano1xx_spi.h"

#include "drv_spi.h"
#include "drv_uart.h"

//#define SPI_DEBUG

static int SPI0_Status = SPI_UNKOWN;
static int SPI1_Status = SPI_UNKOWN;

unsigned int volatile spi0IntFlag = 0;
unsigned int volatile spi1IntFlag = 0;

int DRV_SPI_Init(int spi_port, int speed) {
	int ret = SPI_UNKOWN;
	int retval = 0;
	char msg[128];

	UNLOCKREG();

	memset(msg, 0x00, 128);

	switch(spi_port) {
		case PORT_SPI0:
			if(SPI0_Status == SPI_READY) {
				ret = SPI_READY;
				goto done;
			}

			if(SPI0_Status == SPI_BUSY)	DRV_SPI_Stop(PORT_SPI0);

			//select SPI0 clock source from HCLK
			CLK->CLKSEL2 = (CLK->CLKSEL2 & ~CLK_CLKSEL2_SPI0_MASK) | CLK_CLKSEL2_SPI0_HCLK;

			SPI_Init(SPI0);

			// Enable SPI0 multi function pins
			//	CLK = GPC1
			//	SSO = GPC0
			//	MISO = GPC2
			//	MOSI = GPC3
			GCR->PC_L_MFP = GCR->PC_L_MFP &
						  ~(PC0_MFP_MASK | PC1_MFP_MASK | PC2_MFP_MASK | PC3_MFP_MASK) |
							PC0_MFP_SPI0_SS0   |
							PC1_MFP_SPI0_SCLK  |
							PC2_MFP_SPI0_MISO0 |
							PC3_MFP_SPI0_MOSI0;

#ifdef SPI_DEBUG
			retval = SYS_GetPLLClockFreq();
			memset(msg, 0x00, 128);
			sprintf(msg, " PLL Clock : %d\r\n", retval);
			DRV_UART_Send(msg, strlen(msg));

			retval = SYS_GetHCLKFreq();
			memset(msg, 0x00, 128);
			sprintf(msg, "Host Clock : %d\r\n", retval);
			DRV_UART_Send(msg, strlen(msg));

			memset(msg, 0x00, 128);
			sprintf(msg, "CLK->CLKSEL2 : 0x%08X\r\n", CLK->CLKSEL2);
			DRV_UART_Send(msg, strlen(msg));

			memset(msg, 0x00, 128);
			sprintf(msg, "CLK->CLKSEL2 & CLK_CLKSEL2_SPI0_MASK: 0x%08X\r\n", CLK->CLKSEL2 & CLK_CLKSEL2_SPI0_MASK);
			DRV_UART_Send(msg, strlen(msg));
#endif

			SPI0_Status = SPI_READY;
			ret = SPI_READY;

		    break;

		case PORT_SPI1:
			if(SPI1_Status == SPI_READY) {
				ret = SPI_READY;
				goto done;
			}

			if(SPI1_Status == SPI_BUSY)	DRV_SPI_Stop(PORT_SPI1);

			//select SPI1 clock source from HCLK
			CLK->CLKSEL2 = (CLK->CLKSEL2 & ~CLK_CLKSEL2_SPI1_MASK) | CLK_CLKSEL2_SPI1_HCLK;

			SPI_Init(SPI1);

			// Enable SPI1 multi function pins
			//	CLK = GPC9
			//	SSO = GPC8
			//	MISO = GPC10
			//	MOSI = GPC11
			GCR->PB_L_MFP = GCR->PB_L_MFP &
						  ~(PB0_MFP_MASK | PB1_MFP_MASK | PB2_MFP_MASK | PB3_MFP_MASK) |
						    PB3_MFP_SPI1_SS0   |
						  	PB2_MFP_SPI1_SCLK  |
							PB1_MFP_SPI1_MISO0 |
							PB0_MFP_SPI1_MOSI0;

#ifdef SPI_DEBUG
			retval = SYS_GetPLLClockFreq();
			memset(msg, 0x00, 128);
			sprintf(msg, " PLL Clock : %d\r\n", retval);
			DRV_UART_Send(msg, strlen(msg));

			retval = SYS_GetHCLKFreq();
			memset(msg, 0x00, 128);
			sprintf(msg, "Host Clock : %d\r\n", retval);
			DRV_UART_Send(msg, strlen(msg));

			memset(msg, 0x00, 128);
			sprintf(msg, "CLK->CLKSEL2 : 0x%08X\r\n", CLK->CLKSEL2);
			DRV_UART_Send(msg, strlen(msg));

			memset(msg, 0x00, 128);
			sprintf(msg, "CLK->CLKSEL2 & CLK_CLKSEL2_SPI1_MASK: 0x%08X\r\n", CLK->CLKSEL2 & CLK_CLKSEL2_SPI1_MASK);
			DRV_UART_Send(msg, strlen(msg));
#endif

			SPI1_Status = SPI_READY;
			ret = SPI_READY;

			break;

		default:
			break;
	}

done:
	LOCKREG();

	return ret;
}

int DRV_SPI_Start(int spi_port) {

	return 0;
}

int DRV_SPI_Handler() {

	return 0;
}

unsigned int DRV_SPI_Send(SPI_TypeDef *spi_port, unsigned int dataval, unsigned int datalen) {
	uint32_t spiTxData[2];

	spiTxData[0] = dataval;
	SPI_SetBitLength(spi_port, datalen);
	SPI_SetTxRegister(spi_port, &spiTxData[0], 1);
	SPI_SetGo(spi_port);

	return 0;
}

unsigned int DRV_SPI_Recv(SPI_TypeDef *spi_port, unsigned int *dataval, unsigned int datamask) {
	uint32_t spiRxData[2];

	SPI_DumpRxRegister(spi_port, &spiRxData[0], 1);
	*dataval = (spiRxData[0] & datamask);

	return (spiRxData[0] & datamask);
}

int DRV_SPI_Stop(int spi_port) {
	int ret = SPI_UNKOWN;

	switch(spi_port) {
		case PORT_SPI0:
			if(SPI0_Status == SPI_UNKOWN)	goto done;

			SPI_Close(SPI0);
			SPI_DeInit(SPI0);

			SPI0_Status = SPI_UNKOWN;
			ret = SPI_UNKOWN;

			break;

		case PORT_SPI1:
			if(SPI1_Status == SPI_UNKOWN)	goto done;

			SPI_Close(SPI1);
			SPI_DeInit(SPI1);

			SPI1_Status = SPI_UNKOWN;
			ret = SPI_UNKOWN;

			break;

		default:
			break;
	}

done:

	return ret;
}

int DRV_SPI_isDone(int spi_port) {
	unsigned char msg[32];
	int ret = SPI_UNKOWN;

	switch(spi_port) {
		case PORT_SPI0:
			spi0IntFlag = 0;

			while(1) {
#ifdef SPI_DEBUG
				sprintf(msg, "[SPI] SPI0 Status 0x%08X\r\n", SPI0->STATUS);
				DRV_UART_Send(msg, strlen(msg));
#endif
				if(spi0IntFlag == 1) break;
			}

			spi0IntFlag = 0;
			ret = SPI_DONE;

			break;

		case PORT_SPI1:
			spi1IntFlag = 0;

			while(1) {
#ifdef SPI_DEBUG
				sprintf(msg, "[SPI] SPI1 Status 0x%08X\r\n", SPI1->STATUS);
				DRV_UART_Send(msg, strlen(msg));
#endif
				if(spi1IntFlag == 1) break;
			}

			spi1IntFlag = 0;
			ret = SPI_DONE;

			break;

		default:
			break;
	}

done:

	return ret;
}

void SPI0_IRQHandler(void){
	if( SPI0->STATUS & SPI_STATUS_INTSTS ){ // One transaction done interrupt
		// write '1' to clear SPI0 interrupt flag
		SPI0->STATUS |= SPI_STATUS_INTSTS;
		spi0IntFlag = 1;
	}
}

void SPI1_IRQHandler(void){
	if( SPI1->STATUS & SPI_STATUS_INTSTS ){ // One transaction done interrupt
		// write '1' to clear SPI1 interrupt flag
		SPI1->STATUS |= SPI_STATUS_INTSTS;
		spi1IntFlag = 1;
	}
}
