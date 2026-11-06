#include	"BSP_I2C.h"

static	bspI2CFops_t	*bspI2CFops[4];

I2C_HandleTypeDef	hI2C[4];

u32 BSP_I2C_GetHandle(u8 data)
{
	u32	result = NULL;

	switch(data)
	{
		case BSP_I2C1:
			hI2C[BSP_I2C1].Instance	= I2C1;
			result = (u32)&hI2C[BSP_I2C1];
			break;

		case BSP_I2C2:
			hI2C[BSP_I2C2].Instance	= I2C2;
			result = (u32)&hI2C[BSP_I2C2];
			break;

		case BSP_I2C3:
			hI2C[BSP_I2C3].Instance	= I2C3;
			result = (u32)&hI2C[BSP_I2C3];
			break;

		case BSP_I2C4:
			hI2C[BSP_I2C4].Instance	= I2C4;
			result = (u32)&hI2C[BSP_I2C4];
			break;
	}

	return result;
}

u8 BSP_I2C_RegisterInterface(I2C_HandleTypeDef *pHandle, void *pVoid)
{
	u8	result = 1;

	if(pHandle->Instance == I2C1)			bspI2CFops[BSP_I2C1] = (bspI2CFops_t*)pVoid;
	else if(pHandle->Instance == I2C2)		bspI2CFops[BSP_I2C2] = (bspI2CFops_t*)pVoid;
	else if(pHandle->Instance == I2C3)		bspI2CFops[BSP_I2C3] = (bspI2CFops_t*)pVoid;
	else if(pHandle->Instance == I2C4)		bspI2CFops[BSP_I2C4] = (bspI2CFops_t*)pVoid;
	else									result = 0;

	return	result;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef * hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->MspInit != NULL)
	{
		bspI2CFops[cnt]->MspInit();
	}
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef * hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->MspDeinit != NULL)
	{
		bspI2CFops[cnt]->MspDeinit();
	}
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->MasterTxCpltCallback != NULL)
	{
		bspI2CFops[cnt]->MasterTxCpltCallback();
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->MasterRxCpltCallback != NULL)
	{
		bspI2CFops[cnt]->MasterRxCpltCallback();
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->SlaveTxCpltCallback != NULL)
	{
		bspI2CFops[cnt]->SlaveTxCpltCallback();
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->SlaveRxCpltCallback != NULL)
	{
		bspI2CFops[cnt]->SlaveRxCpltCallback();
	}
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->AddrCallback != NULL)
	{
		bspI2CFops[cnt]->AddrCallback(TransferDirection, AddrMatchCode);
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->ListenCpltCallback != NULL)
	{
		bspI2CFops[cnt]->ListenCpltCallback();
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->MemTxCpltCallback != NULL)
	{
		bspI2CFops[cnt]->MemTxCpltCallback();
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->MemRxCpltCallback != NULL)
	{
		bspI2CFops[cnt]->MemRxCpltCallback();
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->ErrorCallback != NULL)
	{
		bspI2CFops[cnt]->ErrorCallback();
	}
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
	u8	cnt = 0;

	if(hi2c->Instance == I2C1)			cnt = BSP_I2C1;
	else if(hi2c->Instance == I2C2)		cnt = BSP_I2C2;
	else if(hi2c->Instance == I2C3)		cnt = BSP_I2C3;
	else if(hi2c->Instance == I2C4)		cnt = BSP_I2C4;
	else								return;

	if(bspI2CFops[cnt]->AbortCpltCallback != NULL)
	{
		bspI2CFops[cnt]->AbortCpltCallback();
	}
}
