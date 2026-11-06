#ifndef	_MICROSD_DISKIO_H
#define	_MICROSD_DISKIO_H

extern	Diskio_drvTypeDef	SD_Driver;

extern	char	uSdPath[4];
extern	FATFS	uSdFatFs;
extern	FIL		uSdFile;

#endif	// _MICROSD_DISKIO_H