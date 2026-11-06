/**
  ******************************************************************************
  * @file    LwIP/LwIP_UDP_Server/Src/udp_server.c
  * @author  MCD Application Team
  * @brief   UDP server
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
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

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "udp_server.h"

static	ip_addr_t	udpRcvAdrs;
static	u16_t	udpRcvPort;
static	u8_t	udpConnect;

struct	udp_pcb	*udpPcb;

void udp_server_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	memcpy(&udpRcvAdrs, addr, sizeof(udpRcvAdrs));
	udpRcvPort = port;

	Network_RxData((u8*)p->payload, (u16)p->len);

	pbuf_free(p);
}

void udp_server_connect_ip(ip_addr_t adrs, u16_t port)
{
	if(udpConnect == 1)		return;

	udp_connect(udpPcb, &adrs, port);

	udpConnect = 1;
}

void udp_server_connect()
{
	udp_server_connect_ip(udpRcvAdrs, udpRcvPort);
}

void udp_server_disconnect()
{
	udp_disconnect(udpPcb);

	udpConnect = 0;
}

void udp_server_send_data(u8 *pData, u16 size)
{
	struct pbuf *buf;

	if(udpConnect == 0)
	{
		udp_server_connect();
	}

	buf = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_POOL);

	if(buf != NULL)
	{
		// alloc SUCCESS
		pbuf_take(buf, (u8_t*)pData, size);

		udp_send(udpPcb, buf);
	}

	pbuf_free(buf);
}

void udp_server_init()
{
	err_t err;

	/* Create a new UDP control block  */
	udpPcb = udp_new();

	udpConnect = 0;

	if (udpPcb)
	{
		/* Bind the upcb to the UDP_PORT port */
		/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
		err = udp_bind(udpPcb, IP_ADDR_ANY, networkPort);

		if(err == ERR_OK)
		{
			/* Set a receive callback for the upcb */
			udp_recv(udpPcb, udp_server_receive_callback, NULL);
		}
		else
		{
			udp_remove(udpPcb);
		}
	}
}

