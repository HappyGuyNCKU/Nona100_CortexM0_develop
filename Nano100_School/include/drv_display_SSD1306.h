#ifndef DRV_DISPLAY_SSD1306_H
#define DRV_DISPLAY_SSD1306_H

#define W_DOTS	128
#define H_DOTS	 64

int DRV_DISPLAY_SSD1306_Init();
int DRV_DISPLAY_SSD1306_Start();

int DRV_DISPLAY_SSD1306_On();
int DRV_DISPLAY_SSD1306_Off();
int DRV_DISPLAY_SSD1306_Clear();
int DRV_DISPLAY_SSD1306_Write(unsigned char *databuffer, int datalen, int x_start, int x_end, int y_start, int y_end);

int DRV_DISPLAY_SSD1306_Handler();
int DRV_DISPLAY_SSD1306_Stop();

#endif
