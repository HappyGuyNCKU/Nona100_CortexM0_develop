#include "nano1xx.h"
#include "nano1xx_spi.h"
#include "nano1xx_gpio.h"

#include "drv_spi.h"
#include "drv_uart.h"
#include "drv_display_SSD1306.h"

#include "sys_util.h"

#define SPI_DISPLAY_DONE	 0
#define SPI_DISPLAY_ERROR	-1

#define SSD1306_SETCONTRAST			0x81
#define SSD1306_DISPLAYALLON_RESUME	0xA4
#define SSD1306_DISPLAYALLON		0xA5
#define SSD1306_NORMALDISPLAY		0xA6
#define SSD1306_INVERTDISPLAY		0xA7
#define SSD1306_DISPLAYOFF			0xAE
#define SSD1306_DISPLAYON			0xAF
#define SSD1306_SETDISPLAYOFFSET	0xD3
#define SSD1306_SETCOMPINS			0xDA
#define SSD1306_SETVCOMDETECT		0xDB
#define SSD1306_SETDISPLAYCLOCKDIV	0xD5
#define SSD1306_SETPRECHARGE		0xD9
#define SSD1306_SETMULTIPLEX		0xA8
#define SSD1306_SETLOWCOLUMN		0x00
#define SSD1306_SETHIGHCOLUMN		0x10
#define SSD1306_SETSTARTLINE		0x40
#define SSD1306_MEMORYMODE			0x20
#define SSD1306_COLUMNADDR			0x21
#define SSD1306_PAGEADDR			0x22
#define SSD1306_COMSCANINC			0xC0
#define SSD1306_COMSCANDEC			0xC8
#define SSD1306_SEGREMAP			0xA0
#define SSD1306_CHARGEPUMP			0x8D
#define SSD1306_EXTERNALVCC			0x01
#define SSD1306_SWITCHCAPVCC		0x02
#define SSD1306_ACTIVATE_SCROLL		0x2F
#define SSD1306_DEACTIVATE_SCROLL	0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA				0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL					0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL					0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 	0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 	0x2A

//#define DISPLAY_DEBUG

static SPI_TypeDef *display_spi;
static int display_port;

int DRV_DISPLAY_SSD1306_Init() {
	int retval = 0;
	char msg[128];
	SPI_DATA_T conf;

	//GPIO_SetBit(GPIOA, 13);
	//sys_delay(75180);

	display_spi	= SPI1;
	display_port = PORT_SPI1;

	conf.u32Mode = SPI_MODE_MASTER;
	conf.u32Type = SPI_TYPE0;
	conf.i32BitLength = 32;

	// initial port spi-1
	DRV_SPI_Init(display_port, 2000000);

	// Configure SPI as a master, 32-bit transaction
	SPI_Open(display_spi, &conf);

	// Enable AutoSS
	SPI_DisableAutoSS(display_spi);

	// SPI clock rate 4MHz
	SPI_SetClockFreq(display_spi, 4000000, 0);

#ifdef DISPLAY_DEBUG
	retval = SPI_SetClockFreq(display_spi, 2000000, 0);
	memset(msg, 0x00, 128);
	sprintf(msg, "SPI_SetClockFreq : %d\r\n", retval);
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 128);
	sprintf(msg, "SPI Clock Source : %d\r\n", SPI_GetSourceClockFreq(display_spi));
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 128);
	sprintf(msg, "SPI Clock 1      : %d\r\n", SPI_GetClock1Freq(display_spi));
	DRV_UART_Send(msg, strlen(msg));

	memset(msg, 0x00, 128);
	sprintf(msg, "SPI Clock 2      : %d\r\n", SPI_GetClock2Freq(display_spi));
	DRV_UART_Send(msg, strlen(msg));
#endif

	SPI_DisableInt(display_spi);

	return 0;
}

int DRV_DISPLAY_SSD1306_Start() {
	int i = 0;
	unsigned char msg[32];

	if(SPI_IsBusy(display_spi)== TRUE) {
#ifdef DISPLAY_DEBUG
		memset(msg, 0x00, 32);
		sprintf(msg, "[SPI] spi %d busy.....\r\n", display_port);
		DRV_UART_Send(msg, strlen(msg));
#endif
		return SPI_DISPLAY_ERROR;
	}

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as low for command.

	sys_delay(7518);

	SPI_SetSS(display_spi, SPI_SS0);

	sys_delay(7518);

	DRV_SPI_Send(display_spi, SSD1306_DISPLAYOFF, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETDISPLAYCLOCKDIV, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x80, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETMULTIPLEX, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x3F, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETDISPLAYOFFSET, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x00, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETSTARTLINE | 0x00, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SEGREMAP | 0x01, 8);
	//DRV_SPI_Send(display_spi, SSD1306_SEGREMAP | 0x00, 8);	// for rotate 180 degree
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_COMSCANDEC, 8);
	//DRV_SPI_Send(display_spi, SSD1306_COMSCANINC, 8);			// for rotate 180 degree
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETCOMPINS, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x12, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETCONTRAST, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0xCF, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETPRECHARGE, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0xF1, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_SETVCOMDETECT, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x40, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_DISPLAYALLON_RESUME, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_NORMALDISPLAY, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_MEMORYMODE, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x00, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	//DRV_SPI_Send(display_spi, SSD1306_CHARGEPUMP, 8);
	//while(display_spi->CTL & SPI_CTL_GO_BUSY);

	//DRV_SPI_Send(display_spi, 0x14, 8);
	//while(display_spi->CTL & SPI_CTL_GO_BUSY);

	//DRV_SPI_Send(display_spi, SSD1306_DISPLAYON, 8);
	//while(display_spi->CTL & SPI_CTL_GO_BUSY);

	sys_delay(7518);

	SPI_ClrSS(display_spi, SPI_SS0);

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as low for command.

	return 0;
}

int DRV_DISPLAY_SSD1306_Handler() {

	return 0;
}

int DRV_DISPLAY_SSD1306_Stop() {

	return 0;
}

int DRV_DISPLAY_SSD1306_On() {
	//GPIO_SetBit(GPIOA, 13);
	//sys_delay(75180);

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as low for command.

	SPI_SetSS(display_spi, SPI_SS0);

	DRV_SPI_Send(display_spi, SSD1306_CHARGEPUMP, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x14, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_DISPLAYON, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	SPI_ClrSS(display_spi, SPI_SS0);

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as low for command.

	return 0;
}

int DRV_DISPLAY_SSD1306_Off() {
	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as low for command.

	SPI_SetSS(display_spi, SPI_SS0);

	DRV_SPI_Send(display_spi, SSD1306_DISPLAYOFF, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, SSD1306_CHARGEPUMP, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x10, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	SPI_ClrSS(display_spi, SPI_SS0);

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as low for command.

	//sys_delay(75180);

	//GPIO_ClrBit(GPIOA, 13);

	return 0;
}

int DRV_DISPLAY_SSD1306_Clear() {
	unsigned int k = 0;
	unsigned int i = 0;
	unsigned int len = 0;

	// total data length 128 * 64 / 8 == > 1024bytes
	len = (W_DOTS * H_DOTS) >> 3;

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as Low to Stop Data Transfer

	SPI_SetSS(display_spi, SPI_SS0);

	// Set column start and end address
	DRV_SPI_Send(display_spi, SSD1306_COLUMNADDR, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x00, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x7F, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	// Set page start and end address
	DRV_SPI_Send(display_spi, SSD1306_PAGEADDR, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x00, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x07, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	GPIOA->DOUT |= (1 << 12);	// Set PA12 as High to Start Data Transfer

	for(i = 0; i < len; i++) {
		display_spi->CTL = (display_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		//display_spi->TX0 = 0xFF;
		display_spi->TX0 = 0x00;
		display_spi->CTL |= SPI_CTL_GO_BUSY;
		while(display_spi->CTL & SPI_CTL_GO_BUSY);
	}

	SPI_ClrSS(display_spi, SPI_SS0);

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as Low to Stop Data Transfer

	return 0;
}

int DRV_DISPLAY_SSD1306_Write(unsigned char *databuffer, int datalen, int x_start, int x_end, int y_start, int y_end) {
	unsigned int i = 0;
	unsigned int len = 0;

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as Low to Stop Data Transfer

	SPI_SetSS(display_spi, SPI_SS0);

	DRV_SPI_Send(display_spi, SSD1306_MEMORYMODE, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, 0x00, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	// Set column start and end address
	DRV_SPI_Send(display_spi, SSD1306_COLUMNADDR, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, x_start, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, x_end, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	// Set page start and end address
	DRV_SPI_Send(display_spi, SSD1306_PAGEADDR, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, y_start, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	DRV_SPI_Send(display_spi, y_end, 8);
	while(display_spi->CTL & SPI_CTL_GO_BUSY);

	GPIOA->DOUT |= (1 << 12);	// Set PA12 as High to Start Data Transfer

	for(i = 0; i < datalen; i++) {
		display_spi->CTL = (display_spi->CTL & ~SPI_CTL_TX_BIT_LEN_MASK) | (8 << 3);
		display_spi->TX0 = databuffer[i];
		display_spi->CTL |= SPI_CTL_GO_BUSY;
		while(display_spi->CTL & SPI_CTL_GO_BUSY);
	}

	SPI_ClrSS(display_spi, SPI_SS0);

	GPIOA->DOUT &= ~(1 << 12); // Set PA12 as Low to Stop Data Transfer

	return 0;
}
