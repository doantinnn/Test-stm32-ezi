#include	"BSP_Flash.h"
#include	<string.h>


void BSP_Flash_JumpToApplication(void *pVoid)
{
	uint32_t appBase = (pVoid != NULL) ? (uint32_t)pVoid : BSP_FLASH_APP_START_ADRS;
	bspFlashBootApplication_t *pApp = (bspFlashBootApplication_t*)appBase;

	uint32_t appStack = pApp->stackAdrs;
	uint32_t appEntryAddr = (uint32_t)(pApp->application);

	// 1. Kiểm tra địa chỉ Stack Pointer (MSP) có nằm trong vùng RAM hợp lệ (0x20000000 - 0x20080000)
	if((appStack & 0xFFF00000) != BSP_FLASH_BOOT_STACK_ADRS)
	{
		return;
	}

	// Kiểm tra Reset Handler phải nằm trong vùng Flash Application (0x08080000 - 0x08200000)
	if((appEntryAddr < 0x08080000) || (appEntryAddr > 0x08200000))
	{
		return;
	}

	// 2. Vô hiệu hóa ngắt toàn cục và dừng SysTick
	__disable_irq();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;

	// 3. Xóa cờ ngắt và vô hiệu hóa tất cả ngắt trong NVIC
	for(uint32_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	// 4. Vô hiệu hóa I-Cache & D-Cache nếu đang bật để tránh xung đột bộ nhớ
	if((SCB->CCR & SCB_CCR_IC_Msk) != 0U)
	{
		SCB_DisableICache();
	}
	if((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
	{
		SCB_DisableDCache();
	}

	// 5. Thiết lập Vector Table và rào cản bộ nhớ cho Cortex-M7
	SCB->VTOR = appBase;
	__DSB();
	__ISB();

	// 6. Thiết lập MSP
	__set_MSP(appStack);
	__DSB();
	__ISB();

	// 7. Nhảy đến hàm Reset Handler của Application
	void (*appEntry)(void) = (void (*)(void))appEntryAddr;
	appEntry();
}

u8 BSP_Flash_Unlock()
{
	u8	result = 0;
	HAL_StatusTypeDef	rtn;

	rtn = HAL_FLASH_Unlock();

	if(rtn == HAL_OK)	result = 1;

	return result;
}

u8 BSP_Flash_Lock()
{
	u8	result = 0;
	HAL_StatusTypeDef	rtn;

	rtn = HAL_FLASH_Lock();

	if(rtn == HAL_OK)	result = 1;

	return result;
}

u32 BSP_Flash_GetSector(u32 adrs)
{
	u32 sector;

	if((adrs >= BSP_FLASH_MEMORY_ADRS_0) && (adrs < BSP_FLASH_MEMORY_ADRS_1))			sector = FLASH_SECTOR_0;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_1) && (adrs < BSP_FLASH_MEMORY_ADRS_2))		sector = FLASH_SECTOR_1;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_2) && (adrs < BSP_FLASH_MEMORY_ADRS_3))		sector = FLASH_SECTOR_2;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_3) && (adrs < BSP_FLASH_MEMORY_ADRS_4))		sector = FLASH_SECTOR_3;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_4) && (adrs < BSP_FLASH_MEMORY_ADRS_5))		sector = FLASH_SECTOR_4;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_5) && (adrs < BSP_FLASH_MEMORY_ADRS_6))		sector = FLASH_SECTOR_5;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_6) && (adrs < BSP_FLASH_MEMORY_ADRS_7))		sector = FLASH_SECTOR_6;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_7) && (adrs < BSP_FLASH_MEMORY_ADRS_8))		sector = FLASH_SECTOR_7;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_8) && (adrs < BSP_FLASH_MEMORY_ADRS_9))		sector = FLASH_SECTOR_8;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_9) && (adrs < BSP_FLASH_MEMORY_ADRS_10))	sector = FLASH_SECTOR_9;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_10) && (adrs < BSP_FLASH_MEMORY_ADRS_11))	sector = FLASH_SECTOR_10;
	else if((adrs >= BSP_FLASH_MEMORY_ADRS_11) && (adrs < BSP_FLASH_MEMORY_ADRS_12))	sector = FLASH_SECTOR_11;
	else																				sector = 0xFFFFFFFF;

	return sector;
}

u8 BSP_Flash_UserProgramErase(u32 *adrss, u32 size)
{
	u8	result = 0;
	u32	sectorCnt = 0, sectorError = 0;
	HAL_StatusTypeDef	rtn;
	FLASH_EraseInitTypeDef	fErase;
	u32 adrs;

	if(size == 0)	return result;

	BSP_Flash_Unlock();

	adrs = (u32)adrss;

	sectorCnt = BSP_Flash_GetSector(adrs + size - 1) - BSP_Flash_GetSector(adrs) + 1;

	fErase.TypeErase	= FLASH_TYPEERASE_SECTORS;
	fErase.Banks		= FLASH_BANK_1;
	fErase.Sector		= BSP_Flash_GetSector(adrs);
	fErase.NbSectors	= sectorCnt;
	fErase.VoltageRange	= FLASH_VOLTAGE_RANGE_3;

	rtn = HAL_FLASHEx_Erase(&fErase, (uint32_t*)&sectorError);

	BSP_Flash_Lock();

	if(rtn == HAL_OK)	result = 1;

	return result;
}

u8 BSP_Flash_UserProgramWrite(u32 *adrss, u8 *pData, u32 size)
{
	u8	result = 0;
	u32	writeAdrs, writeCnt;
	u32	ableWriteCheck;
	HAL_StatusTypeDef	rtn;
	u32 adrs;

	if(size == 0)				return result;

	adrs = (u32)adrss;

	ableWriteCheck	= BSP_FLASH_MEMORY_ADRS_0 + BSP_FLASH_MEMORY_SIZE;
	ableWriteCheck	-= adrs;

	if(size > ableWriteCheck)	return result;

	BSP_Flash_Unlock();

	writeAdrs	= adrs;
	writeCnt	= 0;

	// Ghi từng Word 32-bit (4 bytes)
	u32 totalWords = (size + 3) / 4;
	u32 *pProgram = (u32*)pData;

	for(writeCnt = 0; writeCnt < totalWords; writeCnt++)
	{
		u32 wordData = 0xFFFFFFFF;
		u32 remainingBytes = size - (writeCnt * 4);

		if(remainingBytes >= 4)
		{
			wordData = pProgram[writeCnt];
		}
		else
		{
			memcpy(&wordData, &pData[writeCnt * 4], remainingBytes);
		}

		rtn = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, writeAdrs, wordData);

		if(rtn != HAL_OK)
		{
			BSP_Flash_Lock();
			return 0;
		}

		writeAdrs += 4;
	}

	BSP_Flash_Lock();
	result = 1;

	return result;
}

u8 BSP_Flash_UserProgramRead(u32 *adrss, u8 *pData, u32 size)
{
	u32 adrs = (u32)adrss;
	memcpy(pData, (u8*)adrs, size);
	return 1;
}

u8 BSP_Flash_UserProgramVerify(u32 *adrss, u8 *pData, u32 size)
{
	u32 adrs = (u32)adrss;
	if(memcmp((u8*)adrs, pData, size) == 0)
	{
		return 1;
	}
	return 0;
}
