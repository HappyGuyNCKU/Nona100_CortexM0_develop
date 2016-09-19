#ifndef DRV_FLASH_SST25VF032B_H
#define DRV_FLASH_SST25VF032B_H

// SST25VF032B is 24bit address, 4Mbytes = 0x400000
#define MAX_ADDRESS	0x3FFFFF
#define MIN_ADDRESS	0x000000

int DRV_FLASH_SST25VF032B_Init();
int DRV_FLASH_SST25VF032B_Start();
int DRV_FLASH_SST25VF032B_Handler();
int DRV_FLASH_SST25VF032B_Stop();

int DRV_FLASH_SST25VF032B_ChipErase();
int DRV_FLASH_SST25VF032B_4KErase(unsigned int address);
int DRV_FLASH_SST25VF032B_32KkErase(unsigned int address);
int DRV_FLASH_SST25VF032B_64KkErase(unsigned int address);

int DRV_FLASH_SST25VF032B_ReadBytes(unsigned int address, unsigned char *databuffer, unsigned int datalen);
int DRV_FLASH_SST25VF032B_WriteBytes(unsigned int address, unsigned char *databuffer, unsigned int datalen);

int DRV_FLASH_SST25VF032B_DisableProtection();
int DRV_FLASH_SST25VF032B_EnableProtection();

#endif
