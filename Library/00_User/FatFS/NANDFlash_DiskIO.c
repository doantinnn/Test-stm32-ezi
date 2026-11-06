#include	<string.h>
#include	"FatFS_GenDrv.h"

//#include	"MicroSD_DiskIO.h"

#ifndef	SDRAM_NAND_FLASH_BACKUP_ADRS
#define	SDRAM_FATFS_BACKUP_ADRS			(uint32_t)0xC7D80000			// 1MB	0xC7D00000 ~ 0xC7DFFFFF
#endif

#define	NAND_BACKUP_ADRS		SDRAM_NAND_FLASH_BACKUP_ADRS

#define	BLOCK_SIZE			nandHandle.Config.BlockSize

static volatile DSTATUS Stat = STA_NOINIT;

char	uNandPath[4] = {0, 0, 0, 0};
FATFS	uNandFatFs;
FIL		uNandFile;

DSTATUS NAND_initialize (BYTE);
DSTATUS NAND_status (BYTE);
DRESULT NAND_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT NAND_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT NAND_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */
  
const Diskio_drvTypeDef  NAND_Driver =
{
	NAND_initialize,
	NAND_status,
	NAND_read, 
#if  _USE_WRITE == 1
	NAND_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
	NAND_ioctl,
#endif /* _USE_IOCTL == 1 */
};

void NAND_DataChange(uint32_t dstAdrs, const BYTE *pSrcData, uint32_t length)
{
	uint8_t *pDstData;
	uint32_t cnt;

	pDstData = (uint8_t*)dstAdrs;

	for(cnt = 0; cnt < length; cnt++)
	{
		pDstData[cnt] = pSrcData[cnt];
	}
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : not used 
  * @retval DSTATUS: Operation status
  */
DSTATUS NAND_initialize(BYTE lun)
{
	Stat = STA_NOINIT;

	if(nandStat == 1)
	{
		Stat &= ~STA_NOINIT;
	}

	return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS NAND_status(BYTE lun)
{
	uint8_t rtn;
	
	Stat = STA_NOINIT;

	rtn = HAL_NAND_Read_Status(&nandHandle);

	if(rtn == NAND_READY)
	{
		Stat &= ~STA_NOINIT;
	}
	
	return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT NAND_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	uint8_t	rtn;
	NAND_AddressTypeDef	adrs;
	DRESULT res = RES_ERROR;

	rtn = BSP_NAND_AdrsCalculator(sector, &adrs);

	if(rtn == 0)		return res;

	rtn = BSP_Nand_ReadPage(&adrs, (uint8_t*)buff, count);

	if(rtn == HAL_OK)
	{
		res = RES_OK;
	}

	return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT NAND_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	uint8_t rtn;
	NAND_AddressTypeDef	adrs;

	rtn = BSP_NAND_AdrsCalculator(sector, &adrs);

	if(rtn == 0)		return res;

	rtn = BSP_Nand_WritePage(&adrs, (uint8_t*)buff, count);

	if(rtn != HAL_OK)	return res;

	res = RES_OK;
	
	return res;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT NAND_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (cmd)
	{
		/* Make sure that no pending write process */
		case CTRL_SYNC :
			res = RES_OK;
			break;

		/* Get number of sectors on the disk (DWORD) */
		case GET_SECTOR_COUNT :
			*(DWORD*)buff = BSP_Nand_GetSectorCnt();
			res = RES_OK;
			break;

		/* Get R/W sector size (WORD) */
		case GET_SECTOR_SIZE :
			*(WORD*)buff = BSP_Nand_GetSectorSize();
			res = RES_OK;
			break;

		/* Get erase block size in unit of sector (DWORD) */
		case GET_BLOCK_SIZE :
			*(DWORD*)buff = BSP_Nand_GetBlockSize();
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}

	return res;
}
#endif /* _USE_IOCTL == 1 */

