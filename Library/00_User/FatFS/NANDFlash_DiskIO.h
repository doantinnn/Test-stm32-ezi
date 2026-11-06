#ifndef	_NANDFLASH_DISKIO_H
#define	_NANDFLASH_DISKIO_H

extern	Diskio_drvTypeDef	NAND_Driver;

extern	char	uNandPath[4];
extern	FATFS	uNandFatFs;
extern	FIL		uNandFile;

#endif	// _NANDFLASH_DISKIO_H