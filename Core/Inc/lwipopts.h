/**
  ******************************************************************************
  * @file    lwipopts.h
  * @brief   Unified LwIP options configuration for STM32F767 (Bare-Metal / NO_SYS=1)
  ******************************************************************************
  */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/**
 * NO_SYS==1: Bare-metal configuration without RTOS
 */
#define NO_SYS                  1
#define SYS_LIGHTWEIGHT_PROT    0

/* ---------- Memory options ---------- */
#define MEM_ALIGNMENT           4
#define MEM_SIZE                (16 * 1024)

/* ---------- Internal Memory Pool Sizes ---------- */
#define MEMP_NUM_PBUF           16
#define MEMP_NUM_RAW_PCB        4
#define MEMP_NUM_UDP_PCB        8
#define MEMP_NUM_TCP_PCB        8
#define MEMP_NUM_TCP_PCB_LISTEN 4
#define MEMP_NUM_TCP_SEG        16
#define MEMP_NUM_SYS_TIMEOUT    12

/* ---------- Pbuf options ---------- */
#define PBUF_POOL_SIZE          16
#define PBUF_POOL_BUFSIZE       1536

/* ---------- Protocol options ---------- */
#define LWIP_ARP                1
#define ARP_QUEUEING            1
#define LWIP_ETHERNET           1
#define LWIP_UDP                1
#define LWIP_TCP                1
#define LWIP_ICMP               1
#define LWIP_DHCP               0   // Static IP default for motion control reliability
#define LWIP_AUTOIP             0
#define LWIP_IGMP               0
#define LWIP_DNS                0

/* ---------- Callback & Timer Options ---------- */
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK   1

/* ---------- Software Checksum Calculation & Verification (Reliable across all PHY/switches) ---------- */
#define CHECKSUM_GEN_IP         1
#define CHECKSUM_GEN_UDP        1
#define CHECKSUM_GEN_TCP        1
#define CHECKSUM_CHECK_IP       1
#define CHECKSUM_CHECK_UDP      1
#define CHECKSUM_CHECK_TCP      1
#define CHECKSUM_GEN_ICMP       1

/* ---------- Sequential/Socket API disabled in NO_SYS=1 ---------- */
#define LWIP_NETCONN            0
#define LWIP_SOCKET             0

#endif /* __LWIPOPTS_H__ */
