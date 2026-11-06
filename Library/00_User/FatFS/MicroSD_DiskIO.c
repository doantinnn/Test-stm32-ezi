#include	<string.h>
#include	"FatFS_GenDrv.h"

//#include	"MicroSD_DiskIO.h"

#define	BLOCK_SIZE			512

static volatile DSTATUS Stat = STA_NOINIT;

char	uSdPath[4] = {0, 0, 0, 0};
FATFS	uSdFatFs;
FIL		uSdFile;

DSTATUS SD_initialize (BYTE);
DSTATUS SD_status (BYTE);
DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */
  
const Diskio_drvTypeDef  SD_Driver =
{
	SD_initialize,
	SD_status,
	SD_read, 
#if  _USE_WRITE == 1
	SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
	SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : not used 
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_initialize(BYTE lun)
{
	Stat = STA_NOINIT;

	/* Configure the uSD device */
	if(BSP_SD_Init() == MSD_OK)
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
DSTATUS SD_status(BYTE lun)
{
	Stat = STA_NOINIT;

	if(BSP_SD_GetCardState() == MSD_OK)
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
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	uint32_t timeout = 100000;

	if(BSP_SD_ReadBlocks((uint32_t*)buff, (uint32_t) (sector), 	count, SD_DATATIMEOUT) == MSD_OK)
	{
		while(BSP_SD_GetCardState()!= MSD_OK)
		{
			if (timeout-- == 0)
			{
				return RES_ERROR;
			}
		}
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
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
	uint32_t timeout = 100000;

	if(BSP_SD_WriteBlocks(	(uint32_t*)buff, (uint32_t)(sector), count, SD_DATATIMEOUT) == MSD_OK)
	{
		while(BSP_SD_GetCardState()!= MSD_OK)
		{
			if (timeout-- == 0)
			{
				return RES_ERROR;
			}
		}    
		res = RES_OK;
	}

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
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;
	HAL_SD_CardInfoTypeDef CardInfo;
	uint16_t u16Dummy;
	uint32_t u32Dummy;

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (cmd)
	{
		/* Make sure that no pending write process */
		case CTRL_SYNC :
			res = RES_OK;
			break;

		/* Get number of sectors on the disk (DWORD) */
		case GET_SECTOR_COUNT :
			BSP_SD_GetCardInfo(&CardInfo);
			u32Dummy = CardInfo.BlockNbr;
			*(DWORD*)buff = u32Dummy;
			res = RES_OK;
			break;

		/* Get R/W sector size (WORD) */
		case GET_SECTOR_SIZE :
			BSP_SD_GetCardInfo(&CardInfo);
			u16Dummy = (uint16_t)CardInfo.BlockSize;
			*(WORD*)buff = u16Dummy;
			res = RES_OK;
			break;

		/* Get erase block size in unit of sector (DWORD) */
		case GET_BLOCK_SIZE :
			//    BSP_SD_GetCardInfo(&CardInfo);
			//    u32Dummy = CardInfo.LogBlockSize;
			u32Dummy = 1;
			*(DWORD*)buff = u32Dummy;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
	}

	return res;
}
#endif /* _USE_IOCTL == 1 */

__weak DWORD get_fattime(void)
{
	return 0;
}