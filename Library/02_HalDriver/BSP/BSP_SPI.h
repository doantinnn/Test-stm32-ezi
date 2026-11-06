#ifndef	_BSP_SPI_H
#define	_BSP_SPI_H

#include	"UserDefine.h"
#include	"stm32f7xx_hal.h"

extern	SPI_HandleTypeDef	hSPI1, hSPI2, hSPI5;

void SPI1_Init();
void SPI1_Deinit();
void SPI2_Init();
void SPI2_Deinit();
void SPI5_Init();
void SPI5_Deinit();

void SPI_Init(SPI_HandleTypeDef *pHandle);
void SPI_Deinit(SPI_HandleTypeDef *pHandle);

#endif	// _BSP_SPI_H