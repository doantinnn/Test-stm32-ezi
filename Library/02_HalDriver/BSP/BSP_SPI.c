#include	"BSP_SPI.h"

SPI_HandleTypeDef	hSPI1, hSPI2, hSPI5;
static	uint8_t	spi1IoInit = 0, spi2IoInit = 0, spi5IoInit = 0;

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef	gpio;
	
	if(hspi->Instance == SPI1)
	{
		// FPGA
		/*
		FPGA_SPI_nCS		PA4
		FPGA_SPI_SCK		PA5
		FPGA_SPI_MISO		PA6
		FPGA_SPI_MOSI		PB5
		*/

		if(spi1IoInit == 0)
		{
			gpio.Mode			=	GPIO_MODE_OUTPUT_PP;
			gpio.Pull			=	GPIO_PULLUP;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_4;

			HAL_GPIO_Init(GPIOA, &gpio);


			gpio.Mode			=	GPIO_MODE_AF_PP;
			gpio.Pull			=	GPIO_PULLUP;
			gpio.Speed			=	GPIO_SPEED_HIGH;

			gpio.Alternate		=	GPIO_AF5_SPI1;
			
			gpio.Pin			=	GPIO_PIN_5;
			gpio.Pin			|=	GPIO_PIN_6;

			HAL_GPIO_Init(GPIOA, &gpio);
			
			gpio.Pin			=	GPIO_PIN_5;

			HAL_GPIO_Init(GPIOB, &gpio);

			__HAL_RCC_SPI1_CLK_ENABLE();

			spi1IoInit = 1;
		}
	}
	else if(hspi->Instance == SPI2)
	{
		// LCD
		/*
		LCD_SPI_nCS			PB12
		LCD_SPI_SCK			PB13
		LCD_SPI_MISO		PB14
		LCD_SPI_MOSI		PB15
		*/

		if(spi2IoInit == 0)
		{
			gpio.Mode			=	GPIO_MODE_OUTPUT_PP;
			gpio.Pull			=	GPIO_PULLUP;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_12;

			HAL_GPIO_Init(GPIOB, &gpio);


			gpio.Mode			=	GPIO_MODE_AF_PP;
			gpio.Pull			=	GPIO_PULLUP;
			gpio.Speed			=	GPIO_SPEED_HIGH;

			gpio.Alternate		=	GPIO_AF5_SPI2;
			
			gpio.Pin			=	GPIO_PIN_13;
			gpio.Pin			|=	GPIO_PIN_14;
			gpio.Pin			|=	GPIO_PIN_15;

			HAL_GPIO_Init(GPIOB, &gpio);

			__HAL_RCC_SPI2_CLK_ENABLE();

			spi2IoInit = 1;
		}
	}
	else if(hspi->Instance == SPI5)
	{
		// TEST

		if(spi5IoInit == 0)
		{
			gpio.Mode			=	GPIO_MODE_OUTPUT_PP;
			gpio.Pull			=	GPIO_PULLUP;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_6;

			HAL_GPIO_Init(GPIOF, &gpio);


			gpio.Mode			=	GPIO_MODE_AF_PP;
			gpio.Pull			=	GPIO_PULLUP;
			gpio.Speed			=	GPIO_SPEED_HIGH;

			gpio.Alternate		=	GPIO_AF5_SPI5;
			
			gpio.Pin			=	GPIO_PIN_7;
			gpio.Pin			|=	GPIO_PIN_8;
			gpio.Pin			|=	GPIO_PIN_9;

			HAL_GPIO_Init(GPIOF, &gpio);

			__HAL_RCC_SPI5_CLK_ENABLE();

			spi5IoInit = 1;
		}
	}
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef	gpio;
	
	if(hspi->Instance == SPI1)
	{
		// FPGA
		/*
		FPGA_SPI_nCS		PA4
		FPGA_SPI_SCK		PA5
		FPGA_SPI_MISO		PA6
		FPGA_SPI_MOSI		PB5
		*/
		if(spi1IoInit != 0)
		{
			gpio.Mode			=	GPIO_MODE_INPUT;
			gpio.Pull			=	GPIO_NOPULL;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_4;

			HAL_GPIO_Init(GPIOA, &gpio);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);


			gpio.Mode			=	GPIO_MODE_INPUT;
			gpio.Pull			=	GPIO_NOPULL;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_5;
			gpio.Pin			|=	GPIO_PIN_6;

			HAL_GPIO_Init(GPIOA, &gpio);
			
			gpio.Pin			=	GPIO_PIN_5;

			HAL_GPIO_Init(GPIOB, &gpio);

			__HAL_RCC_SPI1_CLK_DISABLE();

			spi1IoInit = 0;
		}
	}
	else if(hspi->Instance == SPI2)
	{
		// LCD
		/*
		LCD_SPI_nCS			PB12
		LCD_SPI_SCK			PB13
		LCD_SPI_MISO		PB14
		LCD_SPI_MOSI		PB15
		*/

		if(spi2IoInit != 0)
		{
			gpio.Mode			=	GPIO_MODE_INPUT;
			gpio.Pull			=	GPIO_NOPULL;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_12;

			HAL_GPIO_Init(GPIOB, &gpio);

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

			gpio.Mode			=	GPIO_MODE_INPUT;
			gpio.Pull			=	GPIO_NOPULL;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_13;
			gpio.Pin			|=	GPIO_PIN_14;
			gpio.Pin			|=	GPIO_PIN_15;

			HAL_GPIO_Init(GPIOB, &gpio);

			__HAL_RCC_SPI2_CLK_DISABLE();

			spi2IoInit = 0;
		}
	}
	else if(hspi->Instance == SPI5)
	{
		// TEST

		if(spi5IoInit != 0)
		{
			gpio.Mode			=	GPIO_MODE_INPUT;
			gpio.Pull			=	GPIO_NOPULL;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_6;

			HAL_GPIO_Init(GPIOF, &gpio);

			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);


			gpio.Mode			=	GPIO_MODE_INPUT;
			gpio.Pull			=	GPIO_NOPULL;
			gpio.Speed			=	GPIO_SPEED_HIGH;
			
			gpio.Pin			=	GPIO_PIN_7;
			gpio.Pin			|=	GPIO_PIN_8;
			gpio.Pin			|=	GPIO_PIN_9;

			HAL_GPIO_Init(GPIOF, &gpio);

			__HAL_RCC_SPI5_CLK_DISABLE();

			spi5IoInit = 0;
		}
	}
}

void SPI1_Init()
{
	hSPI1.Instance						= SPI1;
	hSPI1.Init.Mode						= SPI_MODE_MASTER;
	hSPI1.Init.Direction				= SPI_DIRECTION_2LINES;
	hSPI1.Init.DataSize					= SPI_DATASIZE_8BIT;
	hSPI1.Init.CLKPolarity				= SPI_POLARITY_LOW;
	hSPI1.Init.CLKPhase					= SPI_PHASE_1EDGE;
	hSPI1.Init.NSS						= SPI_NSS_SOFT;
	hSPI1.Init.BaudRatePrescaler		= SPI_BAUDRATEPRESCALER_64;
	hSPI1.Init.FirstBit					= SPI_FIRSTBIT_MSB;
	hSPI1.Init.TIMode					= SPI_TIMODE_DISABLE;
	hSPI1.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLE;
	hSPI1.Init.CRCPolynomial			= 0;		// i don't know.
	hSPI1.Init.CRCLength				= SPI_CRC_LENGTH_DATASIZE;
	hSPI1.Init.NSSPMode					= SPI_NSS_PULSE_DISABLE;

	HAL_SPI_Init(&hSPI1);
}

void SPI1_Deinit()
{
	if(hSPI1.Instance != SPI1)	hSPI1.Instance = SPI1;

	HAL_SPI_DeInit(&hSPI1);
}

void SPI2_Init()
{
	hSPI2.Instance						= SPI2;
	hSPI2.Init.Mode						= SPI_MODE_MASTER;
	hSPI2.Init.Direction				= SPI_DIRECTION_2LINES;
	hSPI2.Init.DataSize					= SPI_DATASIZE_8BIT;
	hSPI2.Init.CLKPolarity				= SPI_POLARITY_LOW;
	hSPI2.Init.CLKPhase					= SPI_PHASE_1EDGE;
	hSPI2.Init.NSS						= SPI_NSS_SOFT;
	hSPI2.Init.BaudRatePrescaler		= SPI_BAUDRATEPRESCALER_16;
	hSPI2.Init.FirstBit					= SPI_FIRSTBIT_MSB;
	hSPI2.Init.TIMode					= SPI_TIMODE_DISABLE;
	hSPI2.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLE;
	hSPI2.Init.CRCPolynomial			= 0;		// i don't know.
	hSPI2.Init.CRCLength				= SPI_CRC_LENGTH_DATASIZE;
	hSPI2.Init.NSSPMode					= SPI_NSS_PULSE_ENABLE;

	HAL_SPI_Init(&hSPI2);
}

void SPI2_Deinit()
{
	if(hSPI2.Instance != SPI2)	hSPI2.Instance = SPI2;

	HAL_SPI_DeInit(&hSPI2);
}

void SPI5_Init()
{
	hSPI5.Instance						= SPI5;
	hSPI5.Init.Mode						= SPI_MODE_MASTER;
	hSPI5.Init.Direction				= SPI_DIRECTION_2LINES;
	hSPI5.Init.DataSize					= SPI_DATASIZE_8BIT;
	hSPI5.Init.CLKPolarity				= SPI_POLARITY_LOW;
	hSPI5.Init.CLKPhase					= SPI_PHASE_1EDGE;
	hSPI5.Init.NSS						= SPI_NSS_SOFT;
	hSPI5.Init.BaudRatePrescaler		= SPI_BAUDRATEPRESCALER_16;
	hSPI5.Init.FirstBit					= SPI_FIRSTBIT_MSB;
	hSPI5.Init.TIMode					= SPI_TIMODE_DISABLE;
	hSPI5.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLE;
	hSPI5.Init.CRCPolynomial			= 0;		// i don't know.
	hSPI5.Init.CRCLength				= SPI_CRC_LENGTH_DATASIZE;
	hSPI5.Init.NSSPMode					= SPI_NSS_PULSE_ENABLE;

	HAL_SPI_Init(&hSPI5);
}

void SPI5_Deinit()
{
	if(hSPI5.Instance != SPI5)	hSPI5.Instance = SPI5;

	HAL_SPI_DeInit(&hSPI5);
}

void SPI_Init(SPI_HandleTypeDef *pHandle)
{
	if(pHandle == &hSPI1)
	{
		SPI1_Init();
	}
	else if(pHandle == &hSPI2)
	{
		SPI2_Init();
	}
	else if(pHandle == &hSPI5)
	{
		SPI5_Init();
	}
}

void SPI_Deinit(SPI_HandleTypeDef *pHandle)
{
	if(pHandle == &hSPI1)
	{
		SPI1_Deinit();
	}
	else if(pHandle == &hSPI2)
	{
		SPI2_Deinit();
	}
	else if(pHandle == &hSPI5)
	{
		SPI5_Deinit();
	}
}