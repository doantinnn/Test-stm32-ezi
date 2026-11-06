/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/ethernetif.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-April-2016
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include	<string.h>
#include	"stm32f7xx_hal.h"
#include	"lwip/opt.h"
#include	"lwip/lwip_timers.h"
#include	"netif/etharp.h"
#include	"ethernetif.h"
#if defined(BOOTLOADER)
#include	"IO_Board_BootLoader.h"
#define sysConfig bootConfig
#else
#include	"IO_Board.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if	defined ( __ICCARM__ )
	#pragma data_alignment=4

	#pragma location=0x2007C000
	__no_init ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB];/* Ethernet Rx DMA Descriptors */

	#pragma location=0x2007C080
	__no_init ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB];/* Ethernet Tx DMA Descriptors */

	#pragma location=0x2007C100
	__no_init uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; /* Ethernet Receive Buffers */

	#pragma location=0x2007D8D0
	__no_init uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; /* Ethernet Transmit Buffers */
#elif defined ( __GNUC__ )
	ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB] __attribute__((aligned(4)));
	ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB] __attribute__((aligned(4)));
	uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((aligned(4)));
	uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__((aligned(4)));
#else
	ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB];
	ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB];
	uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
	uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];
#endif

/* Global Ethernet handle*/
ETH_HandleTypeDef EthHandle;

/* Private function prototypes -----------------------------------------------*/
void ethernetif_input(struct netif *netif);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
                       Ethernet MSP Routines
*******************************************************************************/
/**
  * @brief  Initializes the ETH MSP.
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
	GPIO_InitTypeDef	gpio;

/*
	IO Map (MII Interface)

	MII_CRS				: PA0
	MII_RX_CLK			: PA1
	MII_MDIO			: PA2
	MII_COL				: PA3
	MII_RX_DV			: PA7
	MII_RX_ER			: PB10
	MII_MDC				: PC1
	MII_TXD2			: PC2
	MII_TX_CLK			: PC3
	MII_RXD0			: PC4
	MII_RXD1			: PC5
	MII_TXD3			: PE2
	MII_TX_EN			: PB11
	MII_TXD0			: PB12
	MII_TXD1			: PB13
	MII_RXD2			: PB0
	MII_RXD3			: PB1

	ETH_INT				: PB2
*/
	__HAL_RCC_ETH_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	memset(&gpio, NULL, sizeof(gpio));

	// ETH_INT
	gpio.Mode			= GPIO_MODE_IT_FALLING;
	gpio.Pull			= GPIO_NOPULL;
	gpio.Speed			= GPIO_SPEED_HIGH;

	// GPIOB	: ETH_INT
	gpio.Pin			= GPIO_PIN_2;		// ETH_INT				: PB2

	HAL_GPIO_Init(GPIOB, &gpio);

	HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	gpio.Mode			= GPIO_MODE_AF_PP;
	gpio.Pull			= GPIO_NOPULL;
	gpio.Speed			= GPIO_SPEED_HIGH;
	gpio.Alternate		= GPIO_AF11_ETH;

	// GPIOA	: MII_CRS, MII_RX_CLK, MII_MDIO, MII_COL, MII_RX_DV
	gpio.Pin			=	GPIO_PIN_0;		// MII_CRS				: PA0
	gpio.Pin			|=	GPIO_PIN_1;		// MII_RX_CLK				: PA1
	gpio.Pin			|=	GPIO_PIN_2;		// MII_MDIO				: PA2
	gpio.Pin			|=	GPIO_PIN_3;		// MII_COL				: PA3
	gpio.Pin			|=	GPIO_PIN_7;		// MII_RX_DV				: PA7

	HAL_GPIO_Init(GPIOA, &gpio);


	// GPIOB	: MII_RX_ER
	gpio.Pin			=	GPIO_PIN_10;	// MII_RX_ER				: PB10

	HAL_GPIO_Init(GPIOB, &gpio);


	// GPIOC	: MII_MDC, MII_TXD2, MII_TX_CLK, MII_RXD0, MII_RXD1
	gpio.Pin			=	GPIO_PIN_1;		// MII_MDC				: PC1
	gpio.Pin			|=	GPIO_PIN_2;		// MII_TXD2				: PC2
	gpio.Pin			|=	GPIO_PIN_3;		// MII_TX_CLK				: PC3
	gpio.Pin			|=	GPIO_PIN_4;		// MII_RXD0				: PC4
	gpio.Pin			|=	GPIO_PIN_5;		// MII_RXD1				: PC5

	HAL_GPIO_Init(GPIOC, &gpio);

	// GPIOE	: MII_TXD3
	gpio.Pin			=	GPIO_PIN_2;		// MII_TXD3				: PE2

	HAL_GPIO_Init(GPIOE, &gpio);


	// GPIOB	: MII_TX_EN, MII_TXD0, MII_TXD1
	gpio.Pin			=	GPIO_PIN_11;	// MII_TX_EN				: PB11
	gpio.Pin			|=	GPIO_PIN_12;	// MII_TXD0					: PB12
	gpio.Pin			|=	GPIO_PIN_13;	// MII_TXD1					: PB13

	HAL_GPIO_Init(GPIOB, &gpio);


	// GPIOB	: MII_RXD2, MII_RXD3
	gpio.Pin			=	GPIO_PIN_0;		// MII_RXD2					: PB0
	gpio.Pin			|=	GPIO_PIN_1;		// MII_RXD3					: PB1

	HAL_GPIO_Init(GPIOB, &gpio);
}

/**
  * @brief  Ethernet Rx Transfer completed callback
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	networkRxFlag = 1;
}

/**
  * @brief  Ethernet IRQ Handler
  * @param  None
  * @retval None
  */
void ETHERNET_IRQHandler(void)
{
	HAL_ETH_IRQHandler(&EthHandle);
}

/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH) 
*******************************************************************************/
/**
  * @brief In this function, the hardware should be initialized.
  * Called from ethernetif_init().
  *
  * @param netif the already initialized lwip network interface structure
  *        for this ethernetif
  */
static void low_level_init(struct netif *netif)
{
	uint32_t regvalue = 0;
	uint8_t macaddress[6]= {	sysConfig.macAdrs[0],
								sysConfig.macAdrs[1],
								sysConfig.macAdrs[2],
								sysConfig.macAdrs[3],
								sysConfig.macAdrs[4],
								sysConfig.macAdrs[5] };

	EthHandle.Instance = ETH;  
	EthHandle.Init.MACAddr = macaddress;
	EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
	//EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_DISABLE;
	EthHandle.Init.Speed = ETH_SPEED_100M;
	EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
	EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_MII;
	EthHandle.Init.RxMode = ETH_RXPOLLING_MODE;
	EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;
	EthHandle.Init.PhyAddress = DP83848_PHY_ADDRESS;

	/* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
	if (HAL_ETH_Init(&EthHandle) == HAL_OK)
	{
		/* Set netif link flag */
		netif->flags |= NETIF_FLAG_LINK_UP;
	}

	/* Initialize Tx Descriptors list: Chain Mode */
	HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

	/* Initialize Rx Descriptors list: Chain Mode  */
	HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	netif->hwaddr[0] =	sysConfig.macAdrs[0];
	netif->hwaddr[1] =	sysConfig.macAdrs[1];
	netif->hwaddr[2] = 	sysConfig.macAdrs[2];
	netif->hwaddr[3] = 	sysConfig.macAdrs[3];
	netif->hwaddr[4] =	sysConfig.macAdrs[4];
	netif->hwaddr[5] =	sysConfig.macAdrs[5];

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

	/* Enable MAC and DMA transmission and reception */
	HAL_ETH_Start(&EthHandle);

	/**** Configure PHY to generate an interrupt when Eth Link state changes ****/
	/* Read Register Configuration */
	HAL_ETH_ReadPHYRegister(&EthHandle, PHY_MICR, &regvalue);

	regvalue |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);

	/* Enable Interrupts */
	HAL_ETH_WritePHYRegister(&EthHandle, PHY_MICR, regvalue );

	/* Read Register Configuration */
	HAL_ETH_ReadPHYRegister(&EthHandle, PHY_MISR, &regvalue);

	regvalue |= PHY_MISR_LINK_INT_EN;

	/* Enable Interrupt on change of link status */
	HAL_ETH_WritePHYRegister(&EthHandle, PHY_MISR, regvalue);
}

/**
  * @brief This function should do the actual transmission of the packet. The packet is
  * contained in the pbuf that is passed to the function. This pbuf
  * might be chained.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
  * @return ERR_OK if the packet could be sent
  *         an err_t value if the packet couldn't be sent
  *
  * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
  *       strange results. You might consider waiting for space in the DMA queue
  *       to become availale since the stack doesn't retry to send a packet
  *       dropped because of memory failure (except for the TCP timers).
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  err_t errval;
  struct pbuf *q;
  uint8_t *buffer = (uint8_t *)(EthHandle.TxDesc->Buffer1Addr);
  __IO ETH_DMADescTypeDef *DmaTxDesc;
  uint32_t framelength = 0;
  uint32_t bufferoffset = 0;
  uint32_t byteslefttocopy = 0;
  uint32_t payloadoffset = 0;

  DmaTxDesc = EthHandle.TxDesc;
  bufferoffset = 0;
  
  /* copy frame from pbufs to driver buffers */
  for(q = p; q != NULL; q = q->next)
  {
    /* Is this buffer available? If not, goto error */
    if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
    {
      errval = ERR_USE;
      goto error;
    }
    
    /* Get bytes in current lwIP buffer */
    byteslefttocopy = q->len;
    payloadoffset = 0;
    
    /* Check if the length of data to copy is bigger than Tx buffer size*/
    while( (byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE )
    {
      /* Copy data to Tx buffer*/
      memcpy( (uint8_t*)((uint8_t*)buffer + bufferoffset), (uint8_t*)((uint8_t*)q->payload + payloadoffset), (ETH_TX_BUF_SIZE - bufferoffset) );
      
      /* Point to next descriptor */
      DmaTxDesc = (ETH_DMADescTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);
      
      /* Check if the buffer is available */
      if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
      {
        errval = ERR_USE;
        goto error;
      }
      
      buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);
      
      byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
      payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
      framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
      bufferoffset = 0;
    }
    
    /* Copy the remaining bytes */
    memcpy( (uint8_t*)((uint8_t*)buffer + bufferoffset), (uint8_t*)((uint8_t*)q->payload + payloadoffset), byteslefttocopy );
    bufferoffset = bufferoffset + byteslefttocopy;
    framelength = framelength + byteslefttocopy;
  }
  
  /* Prepare transmit descriptors to give to DMA */ 
  HAL_ETH_TransmitFrame(&EthHandle, framelength);
  
  errval = ERR_OK;
  
error:
  
  /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
  if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
  {
    /* Clear TUS ETHERNET DMA flag */
    EthHandle.Instance->DMASR = ETH_DMASR_TUS;
    
    /* Resume DMA transmission*/
    EthHandle.Instance->DMATPDR = 0;
  }
  return errval;
}

/**
  * @brief Should allocate a pbuf and transfer the bytes of the incoming
  * packet from the interface into the pbuf.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return a pbuf filled with the received packet (including MAC header)
  *         NULL on memory error
  */
static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL;
  struct pbuf *q;
  uint16_t len;
  uint8_t *buffer;
  __IO ETH_DMADescTypeDef *dmarxdesc;
  uint32_t bufferoffset = 0;
  uint32_t payloadoffset = 0;
  uint32_t byteslefttocopy = 0;
  uint32_t i=0;
  
  if (HAL_ETH_GetReceivedFrame(&EthHandle) != HAL_OK)
    return NULL;
  
  /* Obtain the size of the packet and put it into the "len" variable. */
  len = EthHandle.RxFrameInfos.length;
  buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;
  
  if (len > 0)
  {
    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  }
  
  if (p != NULL)
  {
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    bufferoffset = 0;
    
    for(q = p; q != NULL; q = q->next)
    {
      byteslefttocopy = q->len;
      payloadoffset = 0;
      
      /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size */
      while( (byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE )
      {
        /* Copy data to pbuf */
        memcpy( (uint8_t*)((uint8_t*)q->payload + payloadoffset), (uint8_t*)((uint8_t*)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));
        
        /* Point to next descriptor */
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
        buffer = (uint8_t *)(dmarxdesc->Buffer1Addr);
        
        byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
        payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
        bufferoffset = 0;
      }
      
      /* Copy remaining data in pbuf */
      memcpy( (uint8_t*)((uint8_t*)q->payload + payloadoffset), (uint8_t*)((uint8_t*)buffer + bufferoffset), byteslefttocopy);
      bufferoffset = bufferoffset + byteslefttocopy;
    }
  }    
    
  /* Release descriptors to DMA */
  /* Point to first descriptor */
  dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
  /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
  for (i=0; i< EthHandle.RxFrameInfos.SegCount; i++)
  {  
    dmarxdesc->Status |= ETH_DMARXDESC_OWN;
    dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
  }
    
  /* Clear Segment_Count */
  EthHandle.RxFrameInfos.SegCount =0;
  
  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)  
  {
    /* Clear RBUS ETHERNET DMA flag */
    EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
    /* Resume DMA reception */
    EthHandle.Instance->DMARPDR = 0;
  }
  return p;
}

/**
  * @brief This function should be called when a packet is ready to be read
  * from the interface. It uses the function low_level_input() that
  * should handle the actual reception of bytes from the network
  * interface. Then the type of the received packet is determined and
  * the appropriate input function is called.
  *
  * @param netif the lwip network interface structure for this ethernetif
  */
void ethernetif_input(struct netif *netif)
{
	err_t err;
	struct pbuf *p;

	/* move received packet into a new pbuf */
	p = low_level_input(netif);

	/* no packet could be read, silently ignore this */
	if (p == NULL)
	{
		return;
	}

	/* entry point to the LwIP stack */
	err = netif->input(p, netif);

	if (err != ERR_OK)
	{
		LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
		pbuf_free(p);
		p = NULL;
	}
}

/**
  * @brief Should be called at the beginning of the program to set up the
  * network interface. It calls the function low_level_init() to do the
  * actual setup of the hardware.
  *
  * This function should be passed as a parameter to netif_add().
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return ERR_OK if the loopif is initialized
  *         ERR_MEM if private data couldn't be allocated
  *         any other err_t on error
  */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

/**
  * @brief  Returns the current time in milliseconds
  *         when LWIP_TIMERS == 1 and NO_SYS == 1
  * @param  None
  * @retval Current Time value
  */
u32_t sys_now(void)
{
  return HAL_GetTick();
}

/**
  * @brief  This function sets the netif link status.
  * @param  netif: the network interface
  * @retval None
  */
void ethernetif_set_link(struct netif *netif)
{
  uint32_t bsr_val = 0;
  uint32_t phy_addr = 0;
  bool link_ok = false;
  
  /* Scan PHY addresses 0x01 and 0x00 (DP83848 can be strapped to 0x00 or 0x01) */
  for (phy_addr = 0; phy_addr <= 1; phy_addr++)
  {
    EthHandle.Init.PhyAddress = phy_addr;
    /* Bit 2 of PHY_BSR is Latch-Low according to IEEE 802.3. Read twice to clear. */
    HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &bsr_val);
    if (HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &bsr_val) == HAL_OK)
    {
      if ((bsr_val & PHY_LINKED_STATUS) != (uint32_t)RESET)
      {
        link_ok = true;
        break;
      }
    }
  }

  if (link_ok)
  {
    if (!netif_is_link_up(netif))
    {
      netif_set_up(netif);
      netif_set_link_up(netif);
    }
  }
}

/**
  * @brief  Link callback function, this function is called on change of link status
  *         to update low level driver configuration.
* @param  netif: The network interface
  * @retval None
  */
void ethernetif_update_config(struct netif *netif)
{
	uint32_t regvalue = 0;

	if(netif_is_link_up(netif))
	{ 
		/* Read the resolved link speed & duplex from PHY Status Register (PHY_SR) */
		if (HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &regvalue) == HAL_OK)
		{
			/* Configure MAC Duplex Mode */
			if((regvalue & PHY_DUPLEX_STATUS) != (uint32_t)RESET)
			{
				EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;  
			}
			else
			{
				EthHandle.Init.DuplexMode = ETH_MODE_HALFDUPLEX;           
			}
			/* Configure MAC Speed */
			if(regvalue & PHY_SPEED_STATUS)
			{  
				EthHandle.Init.Speed = ETH_SPEED_10M; 
			}
			else
			{   
				EthHandle.Init.Speed = ETH_SPEED_100M;
			}
		}

		/* ETHERNET MAC Re-Configuration */
		HAL_ETH_ConfigMAC(&EthHandle, (ETH_MACInitTypeDef *) NULL);

		/* Restart MAC interface */
		HAL_ETH_Start(&EthHandle);   
	}
	else
	{
		/* Stop MAC interface */
		HAL_ETH_Stop(&EthHandle);
	}

	ethernetif_notify_conn_changed(netif);
}

/**
  * @brief  This function notify user about link status changement.
  * @param  netif: the network interface
  * @retval None
  */
__weak void ethernetif_notify_conn_changed(struct netif *netif)
{
  /* NOTE : This is function clould be implemented in user file 
            when the callback is needed,
  */  
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
