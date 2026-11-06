#ifndef	_BSP_FLASH_H
#define	_BSP_FLASH_H

#include	"stm32f7xx_hal.h"
#include	"UserDefine.h"

typedef	struct{
	u32	stackAdrs;
	void (* application)	(void);
}bspFlashBootApplication_t;

enum{
	BSP_FLASH_MEMORY_ADRS_0			= 0x08000000,					//	32 Kbyte		0x08000000 - 0x08007FFF (Sector 0)
	BSP_FLASH_MEMORY_ADRS_1			= 0x08008000,					//	32 Kbyte		0x08008000 - 0x0800FFFF (Sector 1)
	BSP_FLASH_MEMORY_ADRS_2			= 0x08010000,					//	32 Kbyte		0x08010000 - 0x08017FFF (Sector 2)
	BSP_FLASH_MEMORY_ADRS_3			= 0x08018000,					//	32 Kbyte		0x08018000 - 0x0801FFFF (Sector 3)
	BSP_FLASH_MEMORY_ADRS_4			= 0x08020000,					//	128 Kbyte		0x08020000 - 0x0803FFFF (Sector 4)
	BSP_FLASH_MEMORY_ADRS_5			= 0x08040000,					//	256 Kbyte		0x08040000 - 0x0807FFFF (Sector 5)
	BSP_FLASH_MEMORY_ADRS_6			= 0x08080000,					//	256 Kbyte		0x08080000 - 0x080BFFFF (Sector 6 - APP Start)
	BSP_FLASH_MEMORY_ADRS_7			= 0x080C0000,					//	256 Kbyte		0x080C0000 - 0x080FFFFF (Sector 7)
	BSP_FLASH_MEMORY_ADRS_8			= 0x08100000,					//	256 Kbyte		0x08100000 - 0x0813FFFF (Sector 8)
	BSP_FLASH_MEMORY_ADRS_9			= 0x08140000,					//	256 Kbyte		0x08140000 - 0x0817FFFF (Sector 9)
	BSP_FLASH_MEMORY_ADRS_10		= 0x08180000,					//	256 Kbyte		0x08180000 - 0x081BFFFF (Sector 10)
	BSP_FLASH_MEMORY_ADRS_11		= 0x081C0000,					//	256 Kbyte		0x081C0000 - 0x081FFFFF (Sector 11)
	BSP_FLASH_MEMORY_ADRS_12		= 0x08200000,
};

#define	BSP_FLASH_MEMORY_SIZE			(u32)(2 * 1024 * 1024)
#define	BSP_FLASH_APP_START_ADRS		(u32)0x08080000
#define	BSP_FLASH_BOOT_STACK_ADRS		(u32)0x20000000

void BSP_Flash_JumpToApplication(void *pVoid);
u8 BSP_Flash_Unlock();
u8 BSP_Flash_Lock();
u32 BSP_Flash_GetSector(u32 adrs);
u8 BSP_Flash_UserProgramErase(u32 *adrss, u32 size);
u8 BSP_Flash_UserProgramWrite(u32 *adrss, u8 *pData, u32 size);
u8 BSP_Flash_UserProgramRead(u32 *adrss, u8 *pData, u32 size);
u8 BSP_Flash_UserProgramVerify(u32 *adrss, u8 *pData, u32 size);

#endif	//	_BSP_FLASH_H
