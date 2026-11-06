/**********************  DRIVER FOR SPI CONTROLLER ON ORION**********************

   Filename:    Serialize.c
   Description:  Support to . This files is aimed at giving a basic example of
   SPI Controller to simulate the SPI serial interface.

   Version:    0.2
   Date:       Decemb. 2011
   Authors:    Micron S.r.l. Arzano (Napoli)


   THE FOLLOWING DRIVER HAS BEEN TESTED ON ORION WITH FPGA RELEASE 1.4 NON_DDR.

   Since there is no dedicated QUAD SPI controller on PXA processor, the peripheral is
   synthesized on FPGA and it is interfaced with CPU through memory controller. It is
   implemented on chip select-5 region of PXA processor and communicates with device using
   32bits WRFIFO and RDFIFO. This controller is aligned with  Micron SPI Flash Memory.
   That's mean that in extended, dual and quad mode works only with command of these memory.

   These are list of address for different SPI controller registers:

   			Chip Base  is mapped at 0x16000000

   Register         |     Address          |    Read/Write
                    |                      |
   RXFIFO           | (Chip Base + 0x0)    |      Read
   WRFIFO           | (Chip Base + 0x4)    |      Write
   Control Register | (Chip Base + 0x8)    |      R/W
   Status Register  | (Chip Base + 0xC)    |      Read



********************************************************************************

*******************************************************************************/
#include "Serialize.h"

void ConfigureSpi(SpiConfigOptions opt)
{
	switch(opt)
	{
		case OpsWakeUp:
			//	CS_LOW
//			N25Q_SpiCsCtrl(LOW);
			break;

		case OpsInitTransfer:
			//	NOTHING
			break;

		case OpsEndTransfer:
			//	CS_HIGH
//			N25Q_SpiCsCtrl(HIGH);
			break;

		default:
			break;
	}
}

SPI_STATUS Serialize_SPI(const CharStream* char_stream_send, CharStream* char_stream_recv, SpiConfigOptions optBefore, SpiConfigOptions optAfter)
{
	uint8 *char_send, *char_recv;
	uint32	rx_len = 0, tx_len = 0;
	uint32	rxCnt = 0, txCnt = 0;

	tx_len = char_stream_send->length;
	char_send = char_stream_send->pChar;

	if (NULL_PTR != char_stream_recv)
	{
		rx_len = char_stream_recv->length;
		char_recv = char_stream_recv->pChar;
	}
	
	ConfigureSpi(optBefore);

	if(char_send)
	{
	}

	if(char_recv)
	{
	}

//	SPI_Transmit
	do{
		if((tx_len - txCnt) > 32768)
		{
//			N25Q_SpiTransmitData(&char_send[txCnt], 32768);
			txCnt += 32768;
		}
		else
		{
//			N25Q_SpiTransmitData(&char_send[txCnt], (u16)(tx_len - txCnt));
			
			txCnt += (tx_len - txCnt);
		}
	}while(tx_len > txCnt);
	
//	SPI_Receive
	do{
		if((rx_len - rxCnt) > 32768)
		{
//			N25Q_SpiReceiveData(&char_recv[rxCnt], 32768);
			rxCnt += 32768;
		}
		else
		{
//			N25Q_SpiReceiveData(&char_recv[rxCnt], (u16)(rx_len - rxCnt));
			rxCnt += (rx_len - rxCnt);
		}
	}while(rx_len > rxCnt);

	ConfigureSpi(optAfter);

	return RetSpiSuccess;
}

