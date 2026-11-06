#ifndef	_BSP_I2C_H
#define	_BSP_I2C_H

#include	"stm32f7xx_hal.h"
#include	"UserDefine.h"

typedef	struct{
	void	(* MspInit)					(void);
	void	(* MspDeinit)				(void);
	void	(* MasterTxCpltCallback)	(void);
	void	(* MasterRxCpltCallback)	(void);
	void	(* SlaveTxCpltCallback)		(void);
	void	(* SlaveRxCpltCallback)		(void);
	void	(* AddrCallback)		 	(u8, u16);
	void	(* ListenCpltCallback)		(void);
	void	(* MemTxCpltCallback)		(void);
	void	(* MemRxCpltCallback)		(void);
	void	(* ErrorCallback)			(void);
	void	(* AbortCpltCallback)		(void);
}bspI2CFops_t;

enum{
	BSP_I2C1 = 0,
	BSP_I2C2,
	BSP_I2C3,
	BSP_I2C4,
};

extern	I2C_HandleTypeDef	hI2C[4];

u32 BSP_I2C_GetHandle(u8 data);
u8 BSP_I2C_RegisterInterface(I2C_HandleTypeDef *pHandle, void *pVoid);

/*
extern	I2C_HandleTypeDef	hI2C1, hI2C2, hI2C4;

void BSP_I2C1_Init();
void BSP_I2C1_Deinit();
void BSP_I2C2_Init();
void BSP_I2C2_Deinit();
void BSP_I2C4_Init();
void BSP_I2C4_Deinit();
void I2C_Init(I2C_HandleTypeDef *pHandle);
void I2C_Deinit(I2C_HandleTypeDef *pHandle);
*/
#endif	// _BSP_I2C_H
