#include "nano1xx.h"
#include "nano1xx_spi.h"

#include "drv_spi.h"
#include "drv_uart.h"
#include "drv_flash_SST25VF032B.h"

#define SPI_FLASH_DONE	 0
#define SPI_FLASH_ERROR	-1

#define COM_WRITE_STATUS_REG		0x01
#define COM_BYTE_PROGRAM			0x02
#define COM_WRITE_DISABLE			0x04
#define COM_WRITE_ENABLE			0x06
#define COM_ENABLE_WRITE_STATUS_REG	0x50

#define COM_4KB_BLOCK_ERASE			0x20
#define COM_32KB_BLOCK_ERASE		0x52
#define COM_64KB_BLOCK_ERASE		0xD8
#define COM_CHIP_ERASE				0x60
#define COM_AAI_WORD_PROGRAM		0xAD

#define COM_READ_DATA				0x03
#define COM_READ_STATUS_REG			0x05
#define COM_HI_SPEED_READ			0x0B

#define COM_READ_ID					0x90
#define COM_READ_JEDEC_ID			0x9F
#define COM_ENABLE_SO_BUSY			0x70
#define COM_DISABLE_SO_BUSY			0x80


static SPI_TypeDef *flash_spi;
static int flash_port;

static int DRV_FLASH_SST25VF032B_ReadStatus();
static int DRV_FLASH_SST25VF032B_CheckBusy();

int DRV_FLASH_SST25VF032B_Init() {
	int retval = 0;
	char msg[128];
	SPI_DATA_T conf;

	flash_spi = SPI0;
	flash_port = PORT_SPI0;

	conf.u32Mode = SPI_MODE_MASTER;
	conf.u32Type = SPI_TYPE0;
	conf.i32BitLength = 32;

	// initial port spi-0
	DRV_SPI_Init(flash_port, 2000000);

	// Configure SPI as a master, 32-bit transaction
	SPI_Open(flash_spi, &conf);

	// Enable AutoSS
	SPI_DisableAutoSS(flash_spi);

	// SPI clock rate 2MHz
	SPI_SetClockFreq(flash_spi, 2000000, 0);

#ifdef FLASH_DEBUG
	retval = SPI_SetClockFreq(flash_spi, 2000000, 0);
	memset(msg, 0x00, 128);
	sprintf(msg, "SPI_SetClockFreq : %d\r\n", retval);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 128);
	sprintf(msg, "SPI Clock Source : %d\r\n", SPI_GetSourceClockFreq(flash_spi));
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 128);
	sprintf(msg, "SPI Clock 1      : %d\r\n", SPI_GetClock1Freq(flash_spi));
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 128);
	sprintf(msg, "SPI Clock 2      : %d\r\n", SPI_GetClock2Freq(flash_spi));
	DRV_UART_Send(msg, strlen(msg));
#endif

	SPI_DisableInt(flash_spi);

	return 0;
}

int DRV_FLASH_SST25VF032B_Start() {
	unsigned char msg[32];
	uint32_t retData = 0x00;

	if(SPI_IsBusy(flash_spi)== TRUE) {
#ifdef FLASH_DEBUG
		memset(msg, 0x00, 32);
		sprintf(msg, "[SPI] spi %d busy.....\r\n", flash_port);
		DRV_UART_Send(msg, strlen(msg));
#endif
		return SPI_FLASH_ERROR;
	}

	SPI_SetSS(flash_spi, SPI_SS0);

	// send read id command
	DRV_SPI_Send(flash_spi, COM_READ_ID, 8);
	while(SPI_IsBusy(flash_spi));

	// assign address
	DRV_SPI_Send(flash_spi, 0x000000, 24);
	while(SPI_IsBusy(flash_spi));

	// id (16bit)
	DRV_SPI_Send(flash_spi, 0xFFFF, 16);
	while(SPI_IsBusy(flash_spi));

	DRV_SPI_Recv(flash_spi, &retData, 0xFFFF);

	SPI_ClrSS(flash_spi, SPI_SS0);

	sprintf(msg, "SST25VF032B ID %04X\r\n", retData);
	DRV_UART_Send(msg, strlen(msg));

	return 0;
}

int DRV_FLASH_SST25VF032B_Handler() {

	return 0;
}

int DRV_FLASH_SST25VF032B_Stop() {
	SPI_Close(flash_spi);
	flash_spi = NULL;
	flash_port = PORT_SPI0;

	return 0;
}

int DRV_FLASH_SST25VF032B_ReadBytes(unsigned int address, unsigned char *databuffer, unsigned int datalen){
	unsigned int i = 0;
	unsigned char msg[32];
	uint32_t retData;

	if(SPI_IsBusy(flash_spi)== TRUE)	return SPI_FLASH_ERROR;
	if(databuffer == NULL) 				return SPI_FLASH_ERROR;

	if((address + datalen) >= MAX_ADDRESS) {
		datalen = MAX_ADDRESS - address + 1;
	}

	//SPI_SetSS(flash_spi, SPI_SS0);
	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;

	// command (8bit)
	//DRV_SPI_Send(flash_spi, COM_READ_DATA, 8);
	//while(SPI_IsBusy(flash_spi));
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_READ_DATA;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	// address (24bit)
	//DRV_SPI_Send(flash_spi, (address >> 16) & 0xFF, 8);
	//while(SPI_IsBusy(flash_spi));
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = address >> 16;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	//DRV_SPI_Send(flash_spi, (address >> 8) & 0xFF, 8);
	//while(SPI_IsBusy(flash_spi));
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = (address >> 8) & 0xFF;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	//DRV_SPI_Send(flash_spi, address & 0xFF, 8);
	//while(SPI_IsBusy(flash_spi));
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = address & 0xFF;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	// data (8bit*n)
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	for (i = 0; i < datalen; i++) {
		//DRV_SPI_Send(flash_spi, 0x00, 8);
		flash_spi->TX0 = 0x00;
		flash_spi->CTL |= SPI_CTL_GO_BUSY;
		while(flash_spi->CTL & SPI_CTL_GO_BUSY);
		//while(SPI_IsBusy(flash_spi));

		databuffer[i] = flash_spi->RX0 & 0xFF;
		//SPI_DumpRxRegister(flash_spi, &retData, 1);
		//databuffer[i] = retData & 0xFF;
	}

	//SPI_ClrSS(flash_spi, SPI_SS0);
	flash_spi->SSR &= ~SPI_SS0;

	return datalen;
}

int DRV_FLASH_SST25VF032B_WriteBytes(unsigned int address, unsigned char *databuffer, unsigned int datalen){
	unsigned int i = 0;
	unsigned char msg[32];
	unsigned char *dataptr = NULL;
	uint32_t spiTxData[2];
	uint32_t spiRxData[2];

	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;
	if(databuffer == NULL)			 return SPI_FLASH_ERROR;

	dataptr = databuffer;
	for(i = 0; i < datalen; i++) {
		flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;

		flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		flash_spi->TX0 = COM_WRITE_ENABLE;
		flash_spi->CTL |= SPI_CTL_GO_BUSY;
		while(flash_spi->CTL & SPI_CTL_GO_BUSY);

		flash_spi->SSR &= ~SPI_SS0;

		flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;

		flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		flash_spi->TX0 = COM_BYTE_PROGRAM;
		flash_spi->CTL |= SPI_CTL_GO_BUSY;
		while(flash_spi->CTL & SPI_CTL_GO_BUSY);

		//DRV_SPI_Send(flash_spi, address + i, 24);
		flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (24 << 3);
		flash_spi->TX0 = address + i;
		flash_spi->CTL |= SPI_CTL_GO_BUSY;
		while(flash_spi->CTL & SPI_CTL_GO_BUSY);

		flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		flash_spi->TX0 = *dataptr;
		flash_spi->CTL |= SPI_CTL_GO_BUSY;
		while(flash_spi->CTL & SPI_CTL_GO_BUSY);

		flash_spi->SSR &= ~SPI_SS0;

		flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;

		flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		flash_spi->TX0 = COM_READ_STATUS_REG;
		flash_spi->CTL |= SPI_CTL_GO_BUSY;
		while(flash_spi->CTL & SPI_CTL_GO_BUSY);

		flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		while(1) {
			flash_spi->TX0 = 0xFF;
			flash_spi->CTL |= SPI_CTL_GO_BUSY;
			while(flash_spi->CTL & SPI_CTL_GO_BUSY);

			if((flash_spi->RX0 & 0x01) != 0x01)	break;
		}

		flash_spi->SSR &= ~SPI_SS0;

		dataptr++;
	}

	return datalen;
}

int DRV_FLASH_SST25VF032B_ChipErase() {
	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_WRITE_ENABLE, 8);
	while(SPI_IsBusy(flash_spi));
	SPI_ClrSS(flash_spi, SPI_SS0);

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_CHIP_ERASE, 8);
	while(SPI_IsBusy(flash_spi));
	SPI_ClrSS(flash_spi, SPI_SS0);

	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	return 0;
	}

int DRV_FLASH_SST25VF032B_4KErase(unsigned int address) {
	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_WRITE_ENABLE, 8);
	while(SPI_IsBusy(flash_spi));
	SPI_ClrSS(flash_spi, SPI_SS0);

	SPI_SetSS(flash_spi, SPI_SS0);

	DRV_SPI_Send(flash_spi, COM_4KB_BLOCK_ERASE, 8);
	while(SPI_IsBusy(flash_spi));

	DRV_SPI_Send(flash_spi, address & 0x00FFF000, 24);
	while(SPI_IsBusy(flash_spi));

	SPI_ClrSS(flash_spi, SPI_SS0);

	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	return 0;
}

int DRV_FLASH_SST25VF032B_32KkErase(unsigned int address){
	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_WRITE_ENABLE, 8);
	while(SPI_IsBusy(flash_spi));
	SPI_ClrSS(flash_spi, SPI_SS0);

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_32KB_BLOCK_ERASE, 8);
	while(SPI_IsBusy(flash_spi));

	DRV_SPI_Send(flash_spi, address & 0x00FF8000, 24);
	while(SPI_IsBusy(flash_spi));

	SPI_ClrSS(flash_spi, SPI_SS0);

	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	return 0;
}

int DRV_FLASH_SST25VF032B_64KkErase(unsigned int address){
	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_WRITE_ENABLE, 8);
	while(SPI_IsBusy(flash_spi));
	SPI_ClrSS(flash_spi, SPI_SS0);

	SPI_SetSS(flash_spi, SPI_SS0);
	DRV_SPI_Send(flash_spi, COM_64KB_BLOCK_ERASE, 8);
	while(SPI_IsBusy(flash_spi));

	DRV_SPI_Send(flash_spi, address & 0x00FF0000, 24);
	while(SPI_IsBusy(flash_spi));

	SPI_ClrSS(flash_spi, SPI_SS0);

	if(SPI_IsBusy(flash_spi)== TRUE) return SPI_FLASH_ERROR;
	return 0;
}

int DRV_FLASH_SST25VF032B_DisableProtection() {
	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_WRITE_ENABLE;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_ENABLE_WRITE_STATUS_REG;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_WRITE_STATUS_REG;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = 0x01;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = 0x82;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_WRITE_DISABLE;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	return 0;
}

int DRV_FLASH_SST25VF032B_EnableProtection() {
	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_WRITE_ENABLE;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_ENABLE_WRITE_STATUS_REG;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_WRITE_STATUS_REG;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = 0x01;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);

	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = 0x1C;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	flash_spi->SSR = (flash_spi->SSR & ~SPI_SSR_SSR_MASK) | SPI_SS0;
	flash_spi->CTL = (flash_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
	flash_spi->TX0 = COM_WRITE_DISABLE;
	flash_spi->CTL |= SPI_CTL_GO_BUSY;
	while(flash_spi->CTL & SPI_CTL_GO_BUSY);
	flash_spi->SSR &= ~SPI_SS0;

	return 0;
}

static int DRV_FLASH_SST25VF032B_CheckBusy() {
	uint32_t spiTxData[2];
	uint32_t spiRxData[2];

	spiTxData[0] = COM_READ_STATUS_REG;
	SPI_SetBitLength(flash_spi, 8);
	SPI_SetTxRegister(flash_spi, &spiTxData[0], 1);
	SPI_SetSS(flash_spi, SPI_SS0);
	SPI_SetGo(flash_spi);
	while(SPI_IsBusy(flash_spi));

	//loop until device is not busy
	spiTxData[0] = 0xFF;
	while(1) {
		SPI_SetBitLength(flash_spi, 8);
		SPI_SetTxRegister(flash_spi, &spiTxData[0], 1);
		SPI_SetGo(flash_spi);
		while(SPI_IsBusy(flash_spi));

		SPI_DumpRxRegister(flash_spi, &spiRxData[0], 1);
		if((spiRxData[0] & 0x01) != 0x01)	break;
	}

	SPI_ClrSS(flash_spi, SPI_SS0);

	return 0;
}

static int DRV_FLASH_SST25VF032B_ReadStatus() {
	unsigned char msg[32];
	uint32_t retData = 0x00;

	SPI_SetSS(flash_spi, SPI_SS0);

	DRV_SPI_Send(flash_spi, COM_READ_STATUS_REG, 8);
	while(SPI_IsBusy(flash_spi));

	DRV_SPI_Send(flash_spi, 0x00, 8);
	while(SPI_IsBusy(flash_spi));

	DRV_SPI_Recv(flash_spi, &retData, 0xFF);

	SPI_ClrSS(flash_spi, SPI_SS0);

	sprintf(msg, "SST25VF032B Status REG %02X\r\n", retData);
	DRV_UART_Send(msg, strlen(msg));

	return 0;
}
