#include	"BSP_MircoSD.h"

SD_HandleTypeDef hMicroSD1, hMicroSD2;
uint32_t msdBlockSize;
uint32_t cpyData[8192];

void BSP_SD_Memcpy(uint8_t *pDst, uint8_t *pSrc, uint32_t size)
{
	uint32_t cnt;

	for(cnt = 0; cnt < size; cnt++)
	{
		pDst[cnt] = pSrc[cnt];
	}
}

uint8_t BSP_SD_Init(void)
{ 
	return BSP_SD_InitEx(SD_CARD1);
}

uint8_t BSP_SD_InitEx(uint32_t SdCard)
{
	uint8_t sd_state = MSD_OK;
	uint8_t	rtn;

	SD_HandleTypeDef *pSdHandle;
	HAL_SD_CardInfoTypeDef sdCardInfo;

	switch(SdCard)
	{
		case SD_CARD1:
			hMicroSD1.Instance					= SDMMC1;
			hMicroSD1.Init.ClockEdge			= SDMMC_CLOCK_EDGE_RISING;
			hMicroSD1.Init.ClockBypass			= SDMMC_CLOCK_BYPASS_DISABLE;
			hMicroSD1.Init.ClockPowerSave		= SDMMC_CLOCK_POWER_SAVE_DISABLE;
			hMicroSD1.Init.BusWide				= SDMMC_BUS_WIDE_1B;
			hMicroSD1.Init.HardwareFlowControl	= SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
			//hMicroSD1.Init.ClockDiv				= SDMMC_TRANSFER_CLK_DIV;
			hMicroSD1.Init.ClockDiv				= 2;

			pSdHandle = &hMicroSD1;
			break;

		case SD_CARD2:
			hMicroSD2.Instance					= SDMMC2;
			hMicroSD2.Init.ClockEdge			= SDMMC_CLOCK_EDGE_RISING;
			hMicroSD2.Init.ClockBypass			= SDMMC_CLOCK_BYPASS_DISABLE;
			hMicroSD2.Init.ClockPowerSave		= SDMMC_CLOCK_POWER_SAVE_DISABLE;
			hMicroSD2.Init.BusWide				= SDMMC_BUS_WIDE_1B;
			hMicroSD2.Init.HardwareFlowControl	= SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
			hMicroSD2.Init.ClockDiv				= SDMMC_TRANSFER_CLK_DIV;

			pSdHandle = &hMicroSD2;
			break;
	}

	// Detect Pin Init

	// Detect Check
	rtn = BSP_SD_Detect(pSdHandle);

	if(rtn == SD_NOT_PRESENT)	return MSD_ERROR_SD_NOT_PRESENT;

	BSP_SD_MspInit(pSdHandle, NULL);

	if(HAL_SD_Init(pSdHandle) != HAL_OK)
    {
		sd_state = MSD_ERROR;
    }

	if(sd_state == MSD_OK)
	{
		rtn = HAL_SD_ConfigWideBusOperation(pSdHandle, SDMMC_BUS_WIDE_4B);

		if(rtn == HAL_OK)
		{
			sd_state = MSD_OK;
		}
		else
		{
			sd_state = MSD_ERROR;
		}
	}

	HAL_SD_GetCardInfo(pSdHandle, &sdCardInfo);

	msdBlockSize = sdCardInfo.BlockSize;

	return sd_state;
}

uint8_t BSP_SD_DeInit(void)
{
  return BSP_SD_DeInitEx(SD_CARD1);
}

uint8_t BSP_SD_DeInitEx(uint32_t SdCard)
{ 
	uint8_t sd_state = MSD_OK;
	SD_HandleTypeDef *pSdHandle;

	switch(SdCard)
	{
		case SD_CARD1:
			pSdHandle = &hMicroSD1;
			break;

		case SD_CARD2:
			pSdHandle = &hMicroSD2;
			break;
	}

	if((pSdHandle->Instance != SDMMC1) && (pSdHandle->Instance != SDMMC2))
	{
		return sd_state;
	}

	if(HAL_SD_DeInit(pSdHandle) != HAL_OK)
    {
		sd_state = MSD_ERROR;
    }

	BSP_SD_MspDeInit(pSdHandle, NULL);

	return sd_state;
}

uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	return BSP_SD_ReadBlocksEx(SD_CARD1, pData, ReadAddr, NumOfBlocks, Timeout);
}

uint8_t BSP_SD_ReadBlocksEx(uint32_t SdCard, uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	HAL_StatusTypeDef  sd_state = HAL_OK;

	switch(SdCard)
	{
		case SD_CARD1:
			sd_state = HAL_SD_ReadBlocks(&hMicroSD1, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout);
			break;

		case SD_CARD2:
			sd_state = HAL_SD_ReadBlocks(&hMicroSD2, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout);
			break;
	}
	
	if(sd_state == HAL_OK)
	{
		return MSD_OK;
	}
	else
	{
		return MSD_ERROR;
	}
}

uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	uint8_t sd_state;
	uint32_t memSize;
	uint32_t bufferSize;

	memSize = NumOfBlocks * msdBlockSize;

	bufferSize = sizeof(cpyData);

	if(memSize > bufferSize)
	{
		sd_state = MSD_ERROR;
		return sd_state;
	}
	
	BSP_SD_Memcpy((uint8_t*)cpyData, (uint8_t*)pData, memSize);

	sd_state = BSP_SD_WriteBlocksEx(SD_CARD1, cpyData, WriteAddr, NumOfBlocks, Timeout);

	return sd_state;
    //return BSP_SD_WriteBlocksEx(SD_CARD1, pData, WriteAddr, NumOfBlocks, Timeout);
}

uint8_t BSP_SD_WriteBlocksEx(uint32_t SdCard, uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	HAL_StatusTypeDef  sd_state = HAL_OK;

	__disable_irq();

	switch(SdCard)
	{
		case SD_CARD1:
			sd_state = HAL_SD_WriteBlocks(&hMicroSD1, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout);
			break;

		case SD_CARD2:
			sd_state = HAL_SD_WriteBlocks(&hMicroSD2, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout);
			break;
	}

	__enable_irq();
	
	if(sd_state == HAL_OK)
	{
		return MSD_OK;
	}
	else
	{
		return MSD_ERROR;
	}
}

uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	return BSP_SD_ReadBlocks_DMAEx(SD_CARD1, pData, ReadAddr, NumOfBlocks);
}

uint8_t BSP_SD_ReadBlocks_DMAEx(uint32_t SdCard, uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	HAL_StatusTypeDef  sd_state = HAL_OK;

	switch(SdCard)
	{
		case SD_CARD1:
			sd_state = HAL_SD_ReadBlocks_DMA(&hMicroSD1, (uint8_t *)pData, ReadAddr, NumOfBlocks);
			break;

		case SD_CARD2:
			sd_state = HAL_SD_ReadBlocks_DMA(&hMicroSD2, (uint8_t *)pData, ReadAddr, NumOfBlocks);
			break;
	}
	
	if(sd_state == HAL_OK)
	{
		return MSD_OK;
	}
	else
	{
		return MSD_ERROR;
	}
}

uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
	return BSP_SD_WriteBlocks_DMAEx(SD_CARD1, pData, WriteAddr, NumOfBlocks);
}

uint8_t BSP_SD_WriteBlocks_DMAEx(uint32_t SdCard, uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
	HAL_StatusTypeDef  sd_state = HAL_OK;

	switch(SdCard)
	{
		case SD_CARD1:
			sd_state = HAL_SD_WriteBlocks_DMA(&hMicroSD1, (uint8_t *)pData, WriteAddr, NumOfBlocks);
			break;

		case SD_CARD2:
			sd_state = HAL_SD_WriteBlocks_DMA(&hMicroSD2, (uint8_t *)pData, WriteAddr, NumOfBlocks);
			break;
	}
	
	if(sd_state == HAL_OK)
	{
		return MSD_OK;
	}
	else
	{
		return MSD_ERROR;
	}
}

uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
	return BSP_SD_EraseEx(SD_CARD1, StartAddr, EndAddr);
}

uint8_t BSP_SD_EraseEx(uint32_t SdCard, uint32_t StartAddr, uint32_t EndAddr)
{
	HAL_StatusTypeDef  sd_state = HAL_OK;

	switch(SdCard)
	{
		case SD_CARD1:
			sd_state = HAL_SD_Erase(&hMicroSD1, StartAddr, EndAddr);
			break;

		case SD_CARD2:
			sd_state = HAL_SD_Erase(&hMicroSD2, StartAddr, EndAddr); 
			break;
	}
	
	if(sd_state == HAL_OK)
	{
		return MSD_OK;
	}
	else
	{
		return MSD_ERROR;
	}
}

uint8_t BSP_SD_Detect(SD_HandleTypeDef *pHandle)
{
	static uint8_t	sd1IoInit = 0, sd2IoInit = 0;
	uint8_t	result = 0;
	uint8_t	rtn = 1;

	GPIO_InitTypeDef	gpio;

	if(pHandle->Instance == SDMMC1)
	{
		if(sd1IoInit == 0)
		{
			gpio.Pin	= GPIO_PIN_13;
			gpio.Mode	= GPIO_MODE_INPUT;
			gpio.Pull	= GPIO_PULLUP;
			gpio.Speed	= GPIO_SPEED_HIGH;

			HAL_GPIO_Init(GPIOC, &gpio);
			sd1IoInit = 1;
		}

		rtn = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
	}
	else if(pHandle->Instance == SDMMC2)
	{
		if(sd2IoInit == 0)
		{
			/*
			gpio.Pin	= GPIO_PIN_13;
			gpio.Mode	= GPIO_MODE_INPUT;
			gpio.Pull	= GPIO_PULLUP;
			gpio.Speed	= GPIO_SPEED_HIGH;

			HAL_GPIO_Init(GPIOC, &gpio);
			*/
			sd2IoInit = 1;
		}

		// rtn = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
	}

	if(rtn == 0)
	{
		result = SD_PRESENT;
	}
	else
	{
		result = SD_NOT_PRESENT;
	}

	return result;
}

uint8_t	BSP_SD_DetectChk(uint8_t data)
{
	uint8_t	result = SD_NOT_PRESENT;
	SD_HandleTypeDef *pSdHandle;

	switch(data)
	{
		case SD_CARD1:
			pSdHandle = &hMicroSD1;
			break;

		case SD_CARD2:
			pSdHandle = &hMicroSD2;
			break;
	}

	result = BSP_SD_Detect(pSdHandle);

	return result;
}

__weak void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
	static DMA_HandleTypeDef dma_rx_handle;
	static DMA_HandleTypeDef dma_tx_handle;
	static DMA_HandleTypeDef dma_rx_handle2;
	static DMA_HandleTypeDef dma_tx_handle2;  
	GPIO_InitTypeDef gpio_init_structure;

	if(hsd->Instance == SDMMC1)
	{
		/* Enable SDIO clock */
		__HAL_RCC_SDMMC1_CLK_ENABLE();

		/* Enable DMA2 clocks */
		__DMAx_TxRx_CLK_ENABLE();

		/* Enable GPIOs clock */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();

		/* Common GPIO configuration */
		gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
		gpio_init_structure.Pull      = GPIO_PULLUP;
		gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
		gpio_init_structure.Alternate = GPIO_AF12_SDMMC1;

		/* GPIOC configuration: SD1_D0, SD1_D1, SD1_D2, SD1_D3 and SD1_CLK pins */
		gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;

		HAL_GPIO_Init(GPIOC, &gpio_init_structure);

		/* GPIOD configuration: SD1_CMD pin */
		gpio_init_structure.Pin = GPIO_PIN_2;
		HAL_GPIO_Init(GPIOD, &gpio_init_structure);

		/* NVIC configuration for SDIO interrupts */
		HAL_NVIC_SetPriority(SDMMC1_IRQn, 0x0E, 0);
		HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

		dma_rx_handle.Init.Channel             = SD1_DMAx_Rx_CHANNEL;
		dma_rx_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		dma_rx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
		dma_rx_handle.Init.MemInc              = DMA_MINC_ENABLE;
		dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		dma_rx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
		dma_rx_handle.Init.Mode                = DMA_PFCTRL;
		dma_rx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
		dma_rx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
		dma_rx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
		dma_rx_handle.Init.MemBurst            = DMA_MBURST_INC4;
		dma_rx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;
		dma_rx_handle.Instance                 = SD1_DMAx_Rx_STREAM;

		/* Associate the DMA handle */
		__HAL_LINKDMA(hsd, hdmarx, dma_rx_handle);

		/* Deinitialize the stream for new transfer */
		HAL_DMA_DeInit(&dma_rx_handle);

		/* Configure the DMA stream */    
		HAL_DMA_Init(&dma_rx_handle);

		dma_tx_handle.Init.Channel             = SD1_DMAx_Tx_CHANNEL;
		dma_tx_handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
		dma_tx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
		dma_tx_handle.Init.MemInc              = DMA_MINC_ENABLE;
		dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		dma_tx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
		dma_tx_handle.Init.Mode                = DMA_PFCTRL;
		dma_tx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
		dma_tx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
		dma_tx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
		dma_tx_handle.Init.MemBurst            = DMA_MBURST_INC4;
		dma_tx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;
		dma_tx_handle.Instance                 = SD1_DMAx_Tx_STREAM; 

		/* Associate the DMA handle */
		__HAL_LINKDMA(hsd, hdmatx, dma_tx_handle);

		/* Deinitialize the stream for new transfer */
		HAL_DMA_DeInit(&dma_tx_handle);

		/* Configure the DMA stream */
		HAL_DMA_Init(&dma_tx_handle);  

		/* NVIC configuration for DMA transfer complete interrupt */
		HAL_NVIC_SetPriority(SD1_DMAx_Rx_IRQn, 0x0F, 0);
		HAL_NVIC_EnableIRQ(SD1_DMAx_Rx_IRQn);

		/* NVIC configuration for DMA transfer complete interrupt */
		HAL_NVIC_SetPriority(SD1_DMAx_Tx_IRQn, 0x0F, 0);
		HAL_NVIC_EnableIRQ(SD1_DMAx_Tx_IRQn);
	}
	else if(hsd->Instance == SDMMC2)
	{
		/* Enable SDIO clock */
		__HAL_RCC_SDMMC2_CLK_ENABLE();

		/* Enable DMA2 clocks */
		__DMAx_TxRx_CLK_ENABLE();

		/* Enable GPIOs clock */
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();

		/* Common GPIO configuration */
		gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
		gpio_init_structure.Pull      = GPIO_PULLUP;
		gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
		gpio_init_structure.Alternate = GPIO_AF10_SDMMC2;

		/* GPIOB configuration: SD2_D2 and SD2_D3 pins */
		gpio_init_structure.Pin = GPIO_PIN_3 | GPIO_PIN_4;

		HAL_GPIO_Init(GPIOB, &gpio_init_structure);

		/* GPIOD configuration: SD2_CLK and SD2_CMD pins */
		gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		gpio_init_structure.Alternate = GPIO_AF11_SDMMC2;
		HAL_GPIO_Init(GPIOD, &gpio_init_structure);

		/* GPIOG configuration: SD2_D0 and SD2_D1 pins */
		gpio_init_structure.Pin = GPIO_PIN_9 | GPIO_PIN_10;

		HAL_GPIO_Init(GPIOG, &gpio_init_structure);

		/* NVIC configuration for SDIO interrupts */
		HAL_NVIC_SetPriority(SDMMC2_IRQn, 0x0E, 0);
		HAL_NVIC_EnableIRQ(SDMMC2_IRQn);


		dma_rx_handle2.Init.Channel             = SD2_DMAx_Rx_CHANNEL;
		dma_rx_handle2.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		dma_rx_handle2.Init.PeriphInc           = DMA_PINC_DISABLE;
		dma_rx_handle2.Init.MemInc              = DMA_MINC_ENABLE;
		dma_rx_handle2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		dma_rx_handle2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		dma_rx_handle2.Init.Mode                = DMA_PFCTRL;
		dma_rx_handle2.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
		dma_rx_handle2.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
		dma_rx_handle2.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
		dma_rx_handle2.Init.MemBurst            = DMA_MBURST_INC16;
		dma_rx_handle2.Init.PeriphBurst         = DMA_PBURST_INC4;
		dma_rx_handle2.Instance                 = SD2_DMAx_Rx_STREAM;     

		/* Associate the DMA handle */
		__HAL_LINKDMA(hsd, hdmarx, dma_rx_handle2);

		/* Deinitialize the stream for new transfer */
		HAL_DMA_DeInit(&dma_rx_handle2);

		/* Configure the DMA stream */    
		HAL_DMA_Init(&dma_rx_handle2);

		dma_tx_handle2.Init.Channel             = SD2_DMAx_Tx_CHANNEL;
		dma_tx_handle2.Init.Direction           = DMA_MEMORY_TO_PERIPH;
		dma_tx_handle2.Init.PeriphInc           = DMA_PINC_DISABLE;
		dma_tx_handle2.Init.MemInc              = DMA_MINC_ENABLE;
		dma_tx_handle2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		dma_tx_handle2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		dma_tx_handle2.Init.Mode                = DMA_PFCTRL;
		dma_tx_handle2.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
		dma_tx_handle2.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
		dma_tx_handle2.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
		dma_tx_handle2.Init.MemBurst            = DMA_MBURST_INC16;
		dma_tx_handle2.Init.PeriphBurst         = DMA_PBURST_INC4;
		dma_tx_handle2.Instance                 = SD2_DMAx_Tx_STREAM;    

		/* Associate the DMA handle */
		__HAL_LINKDMA(hsd, hdmatx, dma_tx_handle2);

		/* Deinitialize the stream for new transfer */
		HAL_DMA_DeInit(&dma_tx_handle2);

		/* Configure the DMA stream */
		HAL_DMA_Init(&dma_tx_handle2);  

		/* NVIC configuration for DMA transfer complete interrupt */
		HAL_NVIC_SetPriority(SD2_DMAx_Rx_IRQn, 0x0F, 0);
		HAL_NVIC_EnableIRQ(SD2_DMAx_Rx_IRQn);

		/* NVIC configuration for DMA transfer complete interrupt */
		HAL_NVIC_SetPriority(SD2_DMAx_Tx_IRQn, 0x0F, 0);
		HAL_NVIC_EnableIRQ(SD2_DMAx_Tx_IRQn);
	}
}

__weak void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
	static DMA_HandleTypeDef dma_rx_handle;
	static DMA_HandleTypeDef dma_tx_handle;
	static DMA_HandleTypeDef dma_rx_handle2;
	static DMA_HandleTypeDef dma_tx_handle2;

	if(hsd->Instance == SDMMC1)
	{
		/* Disable NVIC for DMA transfer complete interrupts */
		HAL_NVIC_DisableIRQ(SD1_DMAx_Rx_IRQn);
		HAL_NVIC_DisableIRQ(SD1_DMAx_Tx_IRQn);

		/* Deinitialize the stream for new transfer */
		dma_rx_handle.Instance = SD1_DMAx_Rx_STREAM;
		HAL_DMA_DeInit(&dma_rx_handle);

		/* Deinitialize the stream for new transfer */
		dma_tx_handle.Instance = SD1_DMAx_Tx_STREAM;
		HAL_DMA_DeInit(&dma_tx_handle);

		/* Disable NVIC for SDIO interrupts */
		HAL_NVIC_DisableIRQ(SDMMC1_IRQn);

		/* DeInit GPIO pins can be done in the application 
		(by surcharging this __weak function) */

		/* Disable SDMMC1 clock */
		__HAL_RCC_SDMMC1_CLK_DISABLE();
	}
	else if(hsd->Instance == SDMMC2)
	{
		/* Disable NVIC for DMA transfer complete interrupts */
		HAL_NVIC_DisableIRQ(SD2_DMAx_Rx_IRQn);
		HAL_NVIC_DisableIRQ(SD2_DMAx_Tx_IRQn);

		/* Deinitialize the stream for new transfer */
		dma_rx_handle2.Instance = SD2_DMAx_Rx_STREAM;
		HAL_DMA_DeInit(&dma_rx_handle2);

		/* Deinitialize the stream for new transfer */
		dma_tx_handle2.Instance = SD2_DMAx_Tx_STREAM;
		HAL_DMA_DeInit(&dma_tx_handle2);

		/* Disable NVIC for SDIO interrupts */
		HAL_NVIC_DisableIRQ(SDMMC2_IRQn);

		/* DeInit GPIO pins can be done in the application 
		(by surcharging this __weak function) */

		/* Disable SDMMC2 clock */
		__HAL_RCC_SDMMC2_CLK_DISABLE();
	}
	/* GPIO pins clock and DMA clocks can be shut down in the application
	by surcharging this __weak function */ 
}

uint8_t BSP_SD_GetCardState(void)
{
	return BSP_SD_GetCardStateEx(SD_CARD1);
}

uint8_t BSP_SD_GetCardStateEx(uint32_t SdCard)
{
	SD_HandleTypeDef *pSdHandle;
	
	switch(SdCard)
	{
		case SD_CARD1:
			pSdHandle = &hMicroSD1;
			break;

		case SD_CARD2:
			pSdHandle = &hMicroSD2;
			break;
	}

	return((HAL_SD_GetCardState(pSdHandle) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

void BSP_SD_GetCardInfo(BSP_SD_CardInfo *CardInfo)
{
	BSP_SD_GetCardInfoEx(SD_CARD1, CardInfo);
}

void BSP_SD_GetCardInfoEx(uint32_t SdCard, BSP_SD_CardInfo *CardInfo)
{
	switch(SdCard)
	{
		case SD_CARD1:
			HAL_SD_GetCardInfo(&hMicroSD1, CardInfo);
			break;

		case SD_CARD2:
			HAL_SD_GetCardInfo(&hMicroSD2, CardInfo);
			break;
	}
}

void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
//	BSP_SD_AbortCallback((hsd == &hMicroSD1) ? SD_CARD1 : SD_CARD2);
	BSP_SD_AbortCallback(hsd);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
//	BSP_SD_WriteCpltCallback((hsd == &hMicroSD1) ? SD_CARD1 : SD_CARD2);
	BSP_SD_WriteCpltCallback(hsd);
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
//	BSP_SD_ReadCpltCallback((hsd == &hMicroSD1) ? SD_CARD1 : SD_CARD2);
	BSP_SD_ReadCpltCallback(hsd);
}

__weak void BSP_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
	if(hsd->Instance == SDMMC1)
	{
	}
	else if(hsd->Instance == SDMMC2)
	{
	}
}

__weak void BSP_SD_WriteCpltCallback(SD_HandleTypeDef *hsd)
{
	if(hsd->Instance == SDMMC1)
	{
	}
	else if(hsd->Instance == SDMMC2)
	{
	}
}

__weak void BSP_SD_ReadCpltCallback(SD_HandleTypeDef *hsd)
{
	if(hsd->Instance == SDMMC1)
	{
	}
	else if(hsd->Instance == SDMMC2)
	{
	}
}
