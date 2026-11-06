#ifndef	_BSP_FMC_H
#define	_BSP_FMC_H

#include	"stm32f7xx_hal.h"

#define	SDRAM_OK									((uint8_t)0x00)
#define	SDRAM_ERROR									((uint8_t)0x01)

#define	SDRAM_DEVICE_ADDR							((uint32_t)0xC0000000)
//#define	SDRAM_DEVICE_SIZE							((uint32_t)0x1000000)		// 16MB
#define	SDRAM_DEVICE_SIZE							((uint32_t)0x8000000)		// 128MB

#define SDRAM_MEMORY_WIDTH							FMC_SDRAM_MEM_BUS_WIDTH_32

#define SDCLOCK_PERIOD								FMC_SDRAM_CLOCK_PERIOD_2

//#define REFRESH_COUNT								((uint32_t)0x0603)			// Discovery BD
#define REFRESH_COUNT								((uint32_t)824)			// ((64ms / 8192) * 108MHz) - 20 = 823.75

#define SDRAM_TIMEOUT								((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1				((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2				((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4				((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8				((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL			((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED		((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2					((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3					((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD		((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED	((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE		((uint16_t)0x0200)


#define FPGA_DEVICE_ADDR							((uint32_t)0x60000000)
#define FPGA_WRITE_READ_ADDR						((uint32_t)0x00000000)
#define	FPGA_MAX_ADDR								((uint32_t)0x64000000)

#define	FPGA_CMD_DATA								((uint32_t)0x00001000)

#define	FPGA_CMD_MAX								((uint32_t)0x60000FFF)
#define	FPGA_CMD_ADDR								FPGA_DEVICE_ADDR

#define	FPGA_DATA_MAX								((uint32_t)0x60001FFF)
#define	FPGA_DATA_ADDR								(FPGA_DEVICE_ADDR + FPGA_CMD_DATA)

extern	uint8_t	nandStat;
extern	uint8_t	nand16BitEnable;
extern	NAND_HandleTypeDef	nandHandle;

extern	uint32_t nandFullWriteCnt, nandErrorCnt;
extern	uint32_t nandWritePage[2], nandWritePageSize[2];

uint8_t	BSP_SDRAM_Init(void);
uint8_t	BSP_SDRAM_DeInit(void);
void	BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount);
uint8_t	BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t	BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t	BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd);
void	BSP_SDRAM_MspInit(SDRAM_HandleTypeDef  *hsdram, void *Params);
void	BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hsdram, void *Params);

uint32_t BSP_NAND_AdrsCalculator(uint32_t pageNo, NAND_AddressTypeDef *adrs);

void BSP_FMC_Init();
void BSP_Memcpy(void *dst, void *src, uint32_t length);
uint32_t BSP_Nand_GetSectorCnt();
uint32_t BSP_Nand_GetSectorSize();
uint32_t BSP_Nand_GetBlockSize();
HAL_StatusTypeDef BSP_Nand_ReadPage(NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToRead);
HAL_StatusTypeDef BSP_Nand_WritePage(NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToWrite);

#endif	// _BSP_FMC_H