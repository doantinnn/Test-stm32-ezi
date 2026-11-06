/**********************  DRIVER FOR SPI CONTROLLER ON ORION**********************

   Filename:     Serialize.h
   Description:  Header file of Serialize.c
   Version:      0.1
   Date:         Decemb. 2011
   Authors:      Micron S.r.l. Arzano (Napoli)


   THE FOLLOWING DRIVER HAS BEEN TESTED ON ORION WITH FPGA RELEASE 1.4 NON_DDR.

********************************************************************************

   Version History.
   Ver.   Date      Comments

   0.2   Dec 2011  Alpha version

*******************************************************************************/

#ifndef _SERIALIZE_H_
#define _SERIALIZE_H_

#include	"IO_Board.h"

typedef unsigned	char	uint8;
typedef signed		char	sint8;
typedef unsigned	short	uint16;
typedef signed		short	sint16;
typedef unsigned	long	uint32;
typedef signed		long	sint32;

#define NULL_PTR 0x0   // a null pointer

/* Status register masks */
#define SPI_SR1_WIP				(1 << 0)
#define SPI_SR1_WEL				(1 << 1)
#define SPI_SR1_BP0				(1 << 2)
#define SPI_SR1_BP1				(1 << 3)
#define SPI_SR1_BP2				(1 << 4)
#define SPI_SR1_E_FAIL			(1 << 5)
#define SPI_SR1_P_FAIL			(1 << 6)
#define SPI_SR1_SRWD			(1 << 7)

/*Return Type*/

typedef enum{
	RetSpiError,
	RetSpiSuccess
}SPI_STATUS;

typedef unsigned char Bool;

// Acceptable values for SPI master side configuration
typedef enum _SpiConfigOptions{
	OpsNull,  			// do nothing
	OpsWakeUp,			// enable transfer
	OpsInitTransfer,
	OpsEndTransfer,
}SpiConfigOptions;


// char stream definition for
typedef struct _structCharStream{
	uint8* pChar;                                // buffer address that holds the streams
	uint32 length;                               // length of the stream in bytes
}CharStream;

SPI_STATUS Serialize_SPI(const CharStream* char_stream_send, CharStream* char_stream_recv, SpiConfigOptions optBefore, SpiConfigOptions optAfter) ;
void ConfigureSpi(SpiConfigOptions opt);

#endif //end of file

