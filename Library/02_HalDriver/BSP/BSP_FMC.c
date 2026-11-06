#include	"BSP_FMC.h"

SDRAM_HandleTypeDef sdramHandle;
static FMC_SDRAM_TimingTypeDef Timing;
static FMC_SDRAM_CommandTypeDef Command;

uint8_t	nandStat;
uint8_t	nand16BitEnable;
NAND_HandleTypeDef	nandHandle;
static FMC_NAND_PCC_TimingTypeDef	nandTiming;
//static NAND_InfoTypeDef	nandInfo;

SRAM_HandleTypeDef	fpgaHandle;
static FMC_NORSRAM_TimingTypeDef	fpgaTiming;

uint32_t nandFullWriteCnt, nandErrorCnt;
uint32_t nandWritePage[2], nandWritePageSize[2];

void BSP_FMC_MspInit()
{
	GPIO_InitTypeDef	gpio;

	__HAL_RCC_FMC_CLK_ENABLE();
/*
	IO Map

	FMC_DATA0		: PD14
	FMC_DATA1		: PD15
	FMC_DATA2		: PD0
	FMC_DATA3		: PD1
	FMC_DATA4		: PE7
	FMC_DATA5		: PE8
	FMC_DATA6		: PE9
	FMC_DATA7		: PE10
	FMC_DATA8		: PE11
	FMC_DATA9		: PE12
	FMC_DATA10		: PE13
	FMC_DATA11		: PE14
	FMC_DATA12		: PE15
	FMC_DATA13		: PD8
	FMC_DATA14		: PD9
	FMC_DATA15		: PD10
	FMC_DATA16		: PH8
	FMC_DATA17		: PH9
	FMC_DATA18		: PH10
	FMC_DATA19		: PH11
	FMC_DATA20		: PH12
	FMC_DATA21		: PH13
	FMC_DATA22		: PH14
	FMC_DATA23		: PH15
	FMC_DATA24		: PI0
	FMC_DATA25		: PI1
	FMC_DATA26		: PI2
	FMC_DATA27		: PI3
	FMC_DATA28		: PI6
	FMC_DATA29		: PI7
	FMC_DATA30		: PI9
	FMC_DATA31		: PI10

	FMC_ADDR0		: PF0
	FMC_ADDR1		: PF1
	FMC_ADDR2		: PF2
	FMC_ADDR3		: PF3
	FMC_ADDR4		: PF4
	FMC_ADDR5		: PF5
	FMC_ADDR6		: PF12
	FMC_ADDR7		: PF13
	FMC_ADDR8		: PF14
	FMC_ADDR9		: PF15
	FMC_ADDR10		: PG0
	FMC_ADDR11		: PG1
	FMC_ADDR12		: PG2

	FMC_BA0			: PG4
	FMC_BA1			: PG5
	FMC_SDNE0		: PH3
	FMC_SDNWE		: PH5
	FMC_SDNCAS		: PG15
	FMC_SDNRAS		: PF11
	FMC_SDCLK		: PG8
	FMC_SDCKE0		: PH2
	FMC_NBL0		: PE0
	FMC_NBL1		: PE1
	FMC_NBL2		: PI4
	FMC_NBL3		: PI5

	FMC_CLE			: PD11
	FMC_ALE			: PD12
	FMC_NOE			: PD4
	FMC_NWE			: PD5
	FMC_NWAIT		: PD6

	FMC_NCE3		: PG9
	FMC_NE1			: PD7
*/

	gpio.Mode		=	GPIO_MODE_AF_PP;
	gpio.Pull		=	GPIO_PULLUP;
	gpio.Speed		=	GPIO_SPEED_HIGH;
	gpio.Alternate	=	GPIO_AF12_FMC;

	// GPIOD	:	FMC_DATA2, FMC_DATA3, FMC_NOE, FMC_NWE, FMC_NWAIT, FMC_NE1, FMC_DATA13
	//				FMC_DATA14, FMC_DATA15, FMC_CLE, FMC_ALE, FMC_DATA0, FMC_DATA1

	gpio.Pin		=	GPIO_PIN_0;		// FMC_DATA2		: PD0
	gpio.Pin		|=	GPIO_PIN_1;		// FMC_DATA3		: PD1
	gpio.Pin		|=	GPIO_PIN_4;		// FMC_NOE		: PD4
	gpio.Pin		|=	GPIO_PIN_5;		// FMC_NWE		: PD5
	gpio.Pin		|=	GPIO_PIN_6;		// FMC_NWAIT		: PD6
	gpio.Pin		|=	GPIO_PIN_7;		// FMC_NE1		: PD7
	gpio.Pin		|=	GPIO_PIN_8;		// FMC_DATA13		: PD8
	gpio.Pin		|=	GPIO_PIN_9;		// FMC_DATA14		: PD9
	gpio.Pin		|=	GPIO_PIN_10;	// FMC_DATA15		: PD10
	gpio.Pin		|=	GPIO_PIN_11;	// FMC_CLE		: PD11
	gpio.Pin		|=	GPIO_PIN_12;	// FMC_ALE		: PD12
	gpio.Pin		|=	GPIO_PIN_14;	// FMC_DATA0		: PD14
	gpio.Pin		|=	GPIO_PIN_15;	// FMC_DATA1		: PD15

	HAL_GPIO_Init(GPIOD, &gpio);

	// GPIOE	:	FMC_NBL0, FMC_NBL1, FMC_DATA4, FMC_DATA5, FMC_DATA6, FMC_DATA7
	//				FMC_DATA8, FMC_DATA9, FMC_DATA10, FMC_DATA11, FMC_DATA12

	gpio.Pin		=	GPIO_PIN_0;		// FMC_NBL0		: PE0
	gpio.Pin		|=	GPIO_PIN_1;		// FMC_NBL1		: PE1
	gpio.Pin		|=	GPIO_PIN_7;		// FMC_DATA4		: PE7
	gpio.Pin		|=	GPIO_PIN_8;		// FMC_DATA5		: PE8
	gpio.Pin		|=	GPIO_PIN_9;		// FMC_DATA6		: PE9
	gpio.Pin		|=	GPIO_PIN_10;	// FMC_DATA7		: PE10
	gpio.Pin		|=	GPIO_PIN_11;	// FMC_DATA8		: PE11
	gpio.Pin		|=	GPIO_PIN_12;	// FMC_DATA9		: PE12
	gpio.Pin		|=	GPIO_PIN_13;	// FMC_DATA10		: PE13
	gpio.Pin		|=	GPIO_PIN_14;	// FMC_DATA11		: PE14
	gpio.Pin		|=	GPIO_PIN_15;	// FMC_DATA12		: PE15

	HAL_GPIO_Init(GPIOE, &gpio);

	// GPIOF	:	FMC_ADDR0, FMC_ADDR1, FMC_ADDR2, FMC_ADDR3, FMC_ADDR4, FMC_ADDR5
	//				FMC_SDNRAS, FMC_ADDR6, FMC_ADDR7, FMC_ADDR8, FMC_ADDR9

	gpio.Pin		=	GPIO_PIN_0;		// FMC_ADDR0		: PF0
	gpio.Pin		|=	GPIO_PIN_1;		// FMC_ADDR1		: PF1
	gpio.Pin		|=	GPIO_PIN_2;		// FMC_ADDR2		: PF2
	gpio.Pin		|=	GPIO_PIN_3;		// FMC_ADDR3		: PF3
	gpio.Pin		|=	GPIO_PIN_4;		// FMC_ADDR4		: PF4
	gpio.Pin		|=	GPIO_PIN_5;		// FMC_ADDR5		: PF5
	gpio.Pin		|=	GPIO_PIN_11;	// FMC_SDNRAS		: PF11
	gpio.Pin		|=	GPIO_PIN_12;	// FMC_ADDR6		: PF12
	gpio.Pin		|=	GPIO_PIN_13;	// FMC_ADDR7		: PF13
	gpio.Pin		|=	GPIO_PIN_14;	// FMC_ADDR8		: PF14
	gpio.Pin		|=	GPIO_PIN_15;	// FMC_ADDR9		: PF15

	HAL_GPIO_Init(GPIOF, &gpio);

	// GPIOG	:	FMC_ADDR10, FMC_ADDR11, FMC_ADDR12, FMC_BA0, FMC_BA1, FMC_SDCLK
	//				FMC_NCE3, FMC_SDNCAS

	gpio.Pin		=	GPIO_PIN_0;		// FMC_ADDR10		: PG0
	gpio.Pin		|=	GPIO_PIN_1;		// FMC_ADDR11		: PG1
	gpio.Pin		|=	GPIO_PIN_2;		// FMC_ADDR12		: PG2
	gpio.Pin		|=	GPIO_PIN_4;		// FMC_BA0		: PG4
	gpio.Pin		|=	GPIO_PIN_5;		// FMC_BA1		: PG5
	gpio.Pin		|=	GPIO_PIN_8;		// FMC_SDCLK		: PG8
	gpio.Pin		|=	GPIO_PIN_9;		// FMC_NCE3		: PG9
	gpio.Pin		|=	GPIO_PIN_15;	// FMC_SDNCAS		: PG15

	HAL_GPIO_Init(GPIOG, &gpio);

	// GPIOH	:	FMC_SDCKE0, FMC_SDNE0, FMC_SDNWE, FMC_DATA16, FMC_DATA17, FMC_DATA18
	//				FMC_DATA19, FMC_DATA20, FMC_DATA21, FMC_DATA22, FMC_DATA23

	gpio.Pin		=	GPIO_PIN_2;		// FMC_SDCKE0		: PH2
	gpio.Pin		|=	GPIO_PIN_3;		// FMC_SDNE0		: PH3
	gpio.Pin		|=	GPIO_PIN_5;		// FMC_SDNWE		: PH5
	gpio.Pin		|=	GPIO_PIN_8;		// FMC_DATA16		: PH8
	gpio.Pin		|=	GPIO_PIN_9;		// FMC_DATA17		: PH9
	gpio.Pin		|=	GPIO_PIN_10;	// FMC_DATA18		: PH10
	gpio.Pin		|=	GPIO_PIN_11;	// FMC_DATA19		: PH11
	gpio.Pin		|=	GPIO_PIN_12;	// FMC_DATA20		: PH12
	gpio.Pin		|=	GPIO_PIN_13;	// FMC_DATA21		: PH13
	gpio.Pin		|=	GPIO_PIN_14;	// FMC_DATA22		: PH14
	gpio.Pin		|=	GPIO_PIN_15;	// FMC_DATA23		: PH15

	HAL_GPIO_Init(GPIOH, &gpio);

	// GPIOI	:	FMC_DATA24, FMC_DATA25, FMC_DATA26, FMC_DATA27, FMC_NBL2, FMC_NBL3
	//				FMC_DATA28, FMC_DATA29, FMC_DATA30, FMC_DATA31

	gpio.Pin		=	GPIO_PIN_0;		// FMC_DATA24		: PI0
	gpio.Pin		|=	GPIO_PIN_1;		// FMC_DATA25		: PI1
	gpio.Pin		|=	GPIO_PIN_2;		// FMC_DATA26		: PI2
	gpio.Pin		|=	GPIO_PIN_3;		// FMC_DATA27		: PI3
	gpio.Pin		|=	GPIO_PIN_4;		// FMC_NBL2		: PI4
	gpio.Pin		|=	GPIO_PIN_5;		// FMC_NBL3		: PI5
	gpio.Pin		|=	GPIO_PIN_6;		// FMC_DATA28		: PI6
	gpio.Pin		|=	GPIO_PIN_7;		// FMC_DATA29		: PI7
	gpio.Pin		|=	GPIO_PIN_9;		// FMC_DATA30		: PI9
	gpio.Pin		|=	GPIO_PIN_10;	// FMC_DATA31		: PI10

	HAL_GPIO_Init(GPIOI, &gpio);
}

uint8_t BSP_SDRAM_Init(void)
{ 
	static uint8_t sdramstatus = SDRAM_ERROR;
	/* SDRAM device configuration */
	sdramHandle.Instance = FMC_SDRAM_DEVICE;

	/* Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz) */

	Timing.LoadToActiveDelay    = 2;		// tMRD	= 14ns
	Timing.ExitSelfRefreshDelay = 7;		// tXSR	= 67ns
	Timing.SelfRefreshTime      = 4;		// tRAS	= 37ns
	Timing.RowCycleDelay        = 6;		// tRC	= 60ns
	Timing.WriteRecoveryTime    = 2;		// tWR	= 2CLK
	Timing.RPDelay              = 2;		// tRP	= 15ns
	Timing.RCDDelay             = 2;		// tRCD	= 15ns

/*
	Timing.LoadToActiveDelay    = 4;		// tMRD	= 14ns * 2
	Timing.ExitSelfRefreshDelay = 14;		// tXSR	= 67ns * 2
	Timing.SelfRefreshTime      = 8;		// tRAS	= 37ns * 2
	Timing.RowCycleDelay        = 12;		// tRC	= 60ns * 2
	Timing.WriteRecoveryTime    = 4;		// tWR	= 2CLK * 2
	Timing.RPDelay              = 4;		// tRP	= 15ns * 2
	Timing.RCDDelay             = 4;		// tRCD	= 15ns * 2
*/
/*
	Timing.LoadToActiveDelay    = 2;
	Timing.ExitSelfRefreshDelay = 7;
	Timing.SelfRefreshTime      = 4;
	Timing.RowCycleDelay        = 7;
	Timing.WriteRecoveryTime    = 2;
	Timing.RPDelay              = 2;
	Timing.RCDDelay             = 2;
*/

	sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
	sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_10;
	sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
	sdramHandle.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
	sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
	sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	sdramHandle.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_3;
	sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
	sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

	/* SDRAM controller initialization */

	//BSP_SDRAM_MspInit(&sdramHandle, NULL); /* __weak function can be rewritten by the application */

	if(HAL_SDRAM_Init(&sdramHandle, &Timing) != HAL_OK)
	{
		sdramstatus = SDRAM_ERROR;
	}
	else
	{
		sdramstatus = SDRAM_OK;
	}

	/* SDRAM initialization sequence */
	BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);

	return sdramstatus;
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
}

uint8_t BSP_SDRAM_DeInit(void)
{ 
	static uint8_t sdramstatus = SDRAM_ERROR;
	/* SDRAM device de-initialization */
	sdramHandle.Instance = FMC_SDRAM_DEVICE;

	if(HAL_SDRAM_DeInit(&sdramHandle) != HAL_OK)
	{
		sdramstatus = SDRAM_ERROR;
	}
	else
	{
		sdramstatus = SDRAM_OK;
	}

	/* SDRAM controller de-initialization */
	BSP_SDRAM_MspDeInit(&sdramHandle, NULL);

	return sdramstatus;
}

void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
	__IO uint32_t tmpmrd = 0;

	/* Step 1: Configure a clock configuration enable command */
	Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 2: Insert 100 us minimum delay */ 
	/* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
	HAL_Delay(1);

	/* Step 3: Configure a PALL (precharge all) command */ 
	Command.CommandMode            = FMC_SDRAM_CMD_PALL;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);  

	/* Step 4: Configure an Auto Refresh command */ 
	Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 8;
	Command.ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 5: Program the external memory mode register */
	tmpmrd = (uint32_t)	SDRAM_MODEREG_BURST_LENGTH_1          |\
						SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
						SDRAM_MODEREG_CAS_LATENCY_3           |\
						SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
						SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = tmpmrd;

	/* Send the command */
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	/* Step 6: Set the refresh rate counter */
	/* Set the device refresh rate */
	HAL_SDRAM_ProgramRefreshRate(&sdramHandle, RefreshCount); 
}

uint8_t BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
	if(HAL_SDRAM_Read_32b(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	} 
}

uint8_t BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) 
{
	if(HAL_SDRAM_Write_32b(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}

uint8_t BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd)
{
	if(HAL_SDRAM_SendCommand(&sdramHandle, SdramCmd, SDRAM_TIMEOUT) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}

__weak void BSP_SDRAM_MspInit(SDRAM_HandleTypeDef  *hsdram, void *Params)
{

}

__weak void BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hsdram, void *Params)
{
}

uint8_t BSP_NAND_Init()
{
	uint8_t rtn;

	nandHandle.Instance = FMC_NAND_DEVICE;

	nandHandle.Init.NandBank		= FMC_NAND_BANK3;
	nandHandle.Init.Waitfeature		= FMC_NAND_WAIT_FEATURE_ENABLE;
	nandHandle.Init.MemoryDataWidth	= FMC_NAND_MEM_BUS_WIDTH_8;
	nandHandle.Init.EccComputation	= FMC_NAND_ECC_DISABLE;
	nandHandle.Init.ECCPageSize		= FMC_NAND_ECC_PAGE_SIZE_4096BYTE;
	nandHandle.Init.TCLRSetupTime	= 10;
	nandHandle.Init.TARSetupTime	= 10;

	nandHandle.Config.PageSize		= 0;
	nandHandle.Config.SpareAreaSize	= 0;
	nandHandle.Config.BlockSize		= 0;
	nandHandle.Config.BlockNbr		= 0;
	nandHandle.Config.PlaneNbr		= 0;
	nandHandle.Config.PlaneSize		= 0;

	nandHandle.Lock					= HAL_UNLOCKED;

	nandHandle.State				= HAL_NAND_STATE_RESET;
	
	nandTiming.SetupTime			= 8;		// tcs - twp - 1
	nandTiming.WaitSetupTime		= 20;		// (twp or trp) - 1
	nandTiming.HoldSetupTime		= 30;		// tch
	nandTiming.HiZSetupTime			= 16;		// tcs - tds - 1

	rtn = HAL_NAND_Init(&nandHandle, &nandTiming, &nandTiming);

	HAL_NAND_Reset(&nandHandle);

	HAL_Delay(10);

	return rtn;
}

uint8_t	BSP_NAND_Check()
{
	uint8_t result = 0, rtn;
	NAND_IDTypeDef nandID;

	nand16BitEnable = 0;

	rtn = HAL_NAND_Read_ID(&nandHandle, &nandID);

	if(rtn != HAL_OK)				return result;

	// Only Micron SLC NAND Flash

	if(nandID.Maker_Id != 0x2c)		return result;

	switch(nandID.Device_Id)
	{
		case 0xcc:		// 16Bit 4GBit NAND FLASH
			nandHandle.Init.MemoryDataWidth	= FMC_NAND_MEM_BUS_WIDTH_16;

			nandHandle.Config.PageSize				= 1024;	// 2048;
			nandHandle.Config.SpareAreaSize			= 32;	// 64;
			nandHandle.Config.BlockSize				= 64;
			nandHandle.Config.BlockNbr				= 4096;
			nandHandle.Config.PlaneNbr				= 2;
			nandHandle.Config.PlaneSize				= 2048;
			nandHandle.Config.ExtraCommandEnable	= ENABLE;

			nandTiming.SetupTime			= 8;		// tcs - twp - 1
			nandTiming.WaitSetupTime		= 20;		// (twp or trp) - 1
			nandTiming.HoldSetupTime		= 30;		// tch
			nandTiming.HiZSetupTime			= 16;		// tcs - tds - 1

			nand16BitEnable = 1;

			result = 1;
			break;

		case 0xc3:		// 16Bit 8GBit NAND FLASH
			nandHandle.Init.MemoryDataWidth	= FMC_NAND_MEM_BUS_WIDTH_16;

			nandHandle.Config.PageSize				= 2048;	// 4096;
			nandHandle.Config.SpareAreaSize			= 112;	// 224;
			nandHandle.Config.BlockSize				= 64;
			nandHandle.Config.BlockNbr				= 4096;
			nandHandle.Config.PlaneNbr				= 2;
			nandHandle.Config.PlaneSize				= 2048;
			nandHandle.Config.ExtraCommandEnable	= ENABLE;

			nandTiming.SetupTime			= 4;		// tcs - twp - 1
			nandTiming.WaitSetupTime		= 9;		// (twp or trp) - 1
			nandTiming.HoldSetupTime		= 5;		// tch
			nandTiming.HiZSetupTime			= 7;		// tcs - tds - 1

			nand16BitEnable = 1;
			
			result = 1;
			break;

		case 0xc5:		// // 16Bit 16GBit NAND FLASH
			nandHandle.Init.MemoryDataWidth	= FMC_NAND_MEM_BUS_WIDTH_16;

			nandHandle.Config.PageSize				= 2048;	// 4096;
			nandHandle.Config.SpareAreaSize			= 112;	// 224;
			nandHandle.Config.BlockSize				= 64;
			nandHandle.Config.BlockNbr				= 8192;
			nandHandle.Config.PlaneNbr				= 4;
			nandHandle.Config.PlaneSize				= 2048;
			nandHandle.Config.ExtraCommandEnable	= ENABLE;

			nandTiming.SetupTime			= 4;		// tcs - twp - 1
			nandTiming.WaitSetupTime		= 9;		// (twp or trp) - 1
			nandTiming.HoldSetupTime		= 5;		// tch
			nandTiming.HiZSetupTime			= 7;		// tcs - tds - 1

			nand16BitEnable = 1;
			
			result = 1;
			break;

		case 0xd5:		// // 8Bit 16GBit NAND FLASH
			nandHandle.Init.MemoryDataWidth	= FMC_NAND_MEM_BUS_WIDTH_8;

			nandHandle.Config.PageSize				= 4096;	// 4096;
			nandHandle.Config.SpareAreaSize			= 224;	// 224;
			nandHandle.Config.BlockSize				= 64;
			nandHandle.Config.BlockNbr				= 8192;
			nandHandle.Config.PlaneNbr				= 4;
			nandHandle.Config.PlaneSize				= 2048;
			nandHandle.Config.ExtraCommandEnable	= ENABLE;

			nandTiming.SetupTime			= 4;		// tcs - twp - 1
			nandTiming.WaitSetupTime		= 9;		// (twp or trp) - 1
			nandTiming.HoldSetupTime		= 5;		// tch
			nandTiming.HiZSetupTime			= 7;		// tcs - tds - 1

			nand16BitEnable = 0;
			
			result = 1;
			break;
	}

	rtn = HAL_NAND_Init(&nandHandle, &nandTiming, &nandTiming);

	if(rtn != HAL_OK)
	{
		result = 0;
	}

	HAL_NAND_Reset(&nandHandle);

	HAL_Delay(10);

	return result;
}

uint32_t BSP_NAND_AdrsCalculator(uint32_t pageNo, NAND_AddressTypeDef *adrs)
{
	uint32_t result = 0;
	uint32_t pageMaxSize;
	uint32_t blockCnt;

	pageMaxSize = nandHandle.Config.BlockSize;
	
	pageMaxSize *= nandHandle.Config.BlockNbr;

	if(pageNo >= pageMaxSize)	return result;

	adrs->Page	= pageNo % nandHandle.Config.BlockSize;
	
	blockCnt = pageNo / nandHandle.Config.BlockSize;
	
	adrs->Block	= blockCnt % nandHandle.Config.PlaneSize;

	adrs->Plane	= blockCnt / nandHandle.Config.PlaneSize;

	result = 1;
	
	return result;
}

void HAL_NAND_MspInit(NAND_HandleTypeDef *hnand)
{

}

uint8_t BSP_FPGA_Init()
{
	uint8_t	rtn;

	fpgaHandle.Instance					= FMC_NORSRAM_DEVICE;
	fpgaHandle.Extended					= FMC_NORSRAM_EXTENDED_DEVICE;

	fpgaTiming.AddressSetupTime			= 2;
	fpgaTiming.AddressHoldTime			= 3;
	fpgaTiming.DataSetupTime			= 3;
	fpgaTiming.BusTurnAroundDuration	= 2;
	fpgaTiming.CLKDivision				= 2;
	fpgaTiming.DataLatency				= 4;
	fpgaTiming.AccessMode				= FMC_ACCESS_MODE_A;

/*	TEST ONLY
	fpgaTiming.AddressSetupTime			= 5;
	fpgaTiming.AddressHoldTime			= 5;
	fpgaTiming.DataSetupTime			= 5;
	fpgaTiming.BusTurnAroundDuration	= 5;
	fpgaTiming.CLKDivision				= 5;
	fpgaTiming.DataLatency				= 5;
	fpgaTiming.AccessMode				= FMC_ACCESS_MODE_A;
*/
	fpgaHandle.Init.NSBank				= FMC_NORSRAM_BANK1;
	fpgaHandle.Init.DataAddressMux		= FMC_DATA_ADDRESS_MUX_DISABLE;
	fpgaHandle.Init.MemoryType			= FMC_MEMORY_TYPE_SRAM;
	fpgaHandle.Init.MemoryDataWidth		= FMC_NORSRAM_MEM_BUS_WIDTH_32;
	fpgaHandle.Init.BurstAccessMode		= FMC_BURST_ACCESS_MODE_ENABLE;
	fpgaHandle.Init.WaitSignalPolarity	= FMC_WAIT_SIGNAL_POLARITY_LOW;
	fpgaHandle.Init.WaitSignalActive	= FMC_WAIT_TIMING_BEFORE_WS;
	fpgaHandle.Init.WriteOperation		= FMC_WRITE_OPERATION_ENABLE;
	fpgaHandle.Init.WaitSignal			= FMC_WAIT_SIGNAL_DISABLE;
	fpgaHandle.Init.ExtendedMode		= FMC_EXTENDED_MODE_DISABLE;
	fpgaHandle.Init.AsynchronousWait	= FMC_ASYNCHRONOUS_WAIT_DISABLE;
	fpgaHandle.Init.WriteBurst			= FMC_WRITE_BURST_DISABLE;
	fpgaHandle.Init.ContinuousClock		= FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
	fpgaHandle.Init.WriteFifo			= FMC_WRITE_FIFO_DISABLE;
	fpgaHandle.Init.PageSize			= FMC_PAGE_SIZE_NONE;

//	rtn = HAL_SRAM_Init(&fpgaHandle, &fpgaTiming, &fpgaTiming);
	rtn = HAL_SRAM_Init(&fpgaHandle, &fpgaTiming, NULL);

	return rtn;
}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
}

void BSP_FMC_Init()
{
	BSP_FMC_MspInit();

	BSP_SDRAM_Init();

	BSP_FPGA_Init();

	BSP_NAND_Init();

	nandStat = BSP_NAND_Check();
}

#ifndef	SDRAM_NAND_FLASH_BACKUP_ADRS
#define	SDRAM_NAND_FLASH_BACKUP_ADRS			(uint32_t)0xC7D00000			// 512KB	0xC7D00000 ~ 0xC77FFFFF
#endif

uint32_t BSP_Nand_GetSectorCnt()
{
	uint32_t result;

	if(nand16BitEnable)
	{
		result = nandHandle.Config.BlockSize * nandHandle.Config.BlockNbr;
	}
	else
	{
		result = nandHandle.Config.BlockSize * nandHandle.Config.BlockNbr;
	}

	return result;
}

uint32_t BSP_Nand_GetSectorSize()
{
	uint32_t result;

	if(nand16BitEnable)
	{
		result = nandHandle.Config.PageSize * 2;
	}
	else
	{
		result = nandHandle.Config.PageSize;
	}

	return result;
}

uint32_t BSP_Nand_GetBlockSize()
{
	uint32_t result;

	if(nand16BitEnable)
	{
		result = nandHandle.Config.BlockSize;
	}
	else
	{
		result = nandHandle.Config.BlockSize;
	}

	return result;
}

void BSP_Memcpy(void *dst, void *src, uint32_t length)
{
	uint32_t cDst, cSrc;

	cDst = (uint32_t)dst;
	cSrc = (uint32_t)src;

	cDst %= 4;
	cSrc %= 4;
	
	if(((length % 4) == 0) && (cDst == 0) && (cSrc == 0))
	{
		uint32_t *pDst, *pSrc;

		pDst = (uint32_t*)dst;
		pSrc = (uint32_t*)src;

		for(uint32_t cnt = 0; cnt < (length / 4); cnt++)
		{
			pDst[cnt] = pSrc[cnt];
		}
	}
	else
	{
		uint8_t *pDst, *pSrc;

		pDst = (uint8_t*)dst;
		pSrc = (uint8_t*)src;

		for(uint32_t cnt = 0; cnt < length; cnt++)
		{
			pDst[cnt] = pSrc[cnt];
		}
	}
}

HAL_StatusTypeDef BSP_Nand_ReadPage(NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToRead)
{
	HAL_StatusTypeDef rtn;
	
	if(nand16BitEnable)
	{
		rtn = HAL_NAND_Read_Page_16b(&nandHandle, pAddress,(uint16_t*)pBuffer, NumPageToRead);
	}
	else
	{
		rtn = HAL_NAND_Read_Page_8b(&nandHandle, pAddress, pBuffer, NumPageToRead);
	}

	return rtn;
}

HAL_StatusTypeDef BSP_Nand_WritePage(NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToWrite)
{
	HAL_StatusTypeDef rtn;
	uint8_t *pBackup;
	uint16_t savePage;
	uint32_t pageSize;
	uint32_t readCnt;
	uint32_t backupAdrs, changeSize;

	if(NumPageToWrite == 0)		return HAL_OK;

	if((pAddress->Page + NumPageToWrite) > nandHandle.Config.BlockSize)
	{
		nandErrorCnt++;
		return HAL_ERROR;
	}

	if(nandWritePage[0] > pAddress->Page)		nandWritePage[0] = pAddress->Page;
	if(nandWritePage[1] < pAddress->Page)		nandWritePage[1] = pAddress->Page;

	if(nandWritePageSize[0] > NumPageToWrite)	nandWritePageSize[0] = NumPageToWrite;
	if(nandWritePageSize[1] < NumPageToWrite)	nandWritePageSize[1] = NumPageToWrite;

	if((pAddress->Page == 0) && (NumPageToWrite == nandHandle.Config.BlockSize))
	{
		nandFullWriteCnt++;

		rtn = HAL_NAND_Erase_Block(&nandHandle, pAddress);

		if(rtn != HAL_OK)	return HAL_ERROR;
		
		if(nand16BitEnable)
		{
			rtn = HAL_NAND_Write_Page_16b(&nandHandle, pAddress, (uint16_t*)pBuffer, NumPageToWrite);
		}
		else
		{
			rtn = HAL_NAND_Write_Page_8b(&nandHandle, pAddress, pBuffer, NumPageToWrite);
		}
	}
	else
	{
		if(nand16BitEnable)
		{
			pageSize = nandHandle.Config.PageSize * 2;
		}
		else
		{
			pageSize = nandHandle.Config.PageSize;
		}
		
		pBackup = (uint8_t*)SDRAM_NAND_FLASH_BACKUP_ADRS;

		savePage = pAddress->Page;

		pAddress->Page = 0;

		readCnt = nandHandle.Config.BlockSize;

//		DATA LOAD(BACKUP)

		rtn = BSP_Nand_ReadPage(pAddress, pBackup, readCnt);

		if(rtn != HAL_OK)	return HAL_ERROR;

//		DATA ERASE

		rtn = HAL_NAND_Erase_Block(&nandHandle, pAddress);

		if(rtn != HAL_OK)	return HAL_ERROR;

//		DATA MODIFY

		backupAdrs = SDRAM_NAND_FLASH_BACKUP_ADRS;
		backupAdrs += pageSize * savePage;

		changeSize = pageSize * NumPageToWrite;

		BSP_Memcpy((void*)backupAdrs, (void*)pBuffer, changeSize);

//		DATA WRITE

		if(nand16BitEnable)
		{
			rtn = HAL_NAND_Write_Page_16b(&nandHandle, pAddress, (uint16_t*)pBackup, readCnt);
		}
		else
		{
			rtn = HAL_NAND_Write_Page_8b(&nandHandle, pAddress, pBackup, readCnt);
		}
	}

	return rtn;
}
