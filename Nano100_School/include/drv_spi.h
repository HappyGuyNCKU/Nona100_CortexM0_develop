#ifndef DRV_SPI_H
#define DRV_SPI_H

typedef enum {
	PORT_SPI0 = 0,
	PORT_SPI1 = 1
} port_spi_num;

typedef enum {
	SPI_UNKOWN = -1,
	SPI_READY = 0,
	SPI_BUSY = 1,
	SPI_DONE,
} port_spi_status;

int DRV_SPI_Init(int spi_port, int speed);
int DRV_SPI_Start(int spi_port);
int DRV_SPI_Handler();
unsigned int DRV_SPI_Send(SPI_TypeDef *spi_port, unsigned int dataval, unsigned int datalen);
unsigned int DRV_SPI_Recv(SPI_TypeDef *spi_port, unsigned int *dataval, unsigned int datamask);
int DRV_SPI_Stop(int spi_port);
int DRV_SPI_isDone(int spi_port);

#endif
