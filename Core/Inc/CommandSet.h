/**
  ******************************************************************************
  * @file    CommandSet.h
  * @brief   Command Processing Engine Architecture Header
  *          Referenced from TOP Engineering PG_StudyProject_Test CommandSet
  ******************************************************************************
  */

#ifndef _COMMAND_SET_H_
#define _COMMAND_SET_H_

#include "UserDefine.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMDSET_COMM_SRC_COUNT       4
#define CMDSET_MAX_COUNT            32
#define CMDSET_RX_BUF_SIZE          1024
#define CMDSET_TX_BUF_SIZE          2048

/* Communication Source Types */
enum {
    CMD_COMM_SRC_TYPE_UART     = 0,  /* RS-232 USART1 on PA9/PA10 */
    CMD_COMM_SRC_TYPE_ETHERNET = 1,  /* UDP Server Port 5000 for PC UI */
    CMD_COMM_SRC_TYPE_INTERNAL = 2,  /* Internal Script or Test */
};

/* Command Execution Mode Flags */
enum {
    CMD_MODE_NONE        = 0x00000000,
    CMD_MODE_ONCE        = 0x00000001,
    CMD_MODE_DEACTIVATE  = 0x00000002,
    CMD_MODE_RESULT_OUT  = 0x00000004,
};

/* Forward Declarations */
typedef struct commandList_s commandList_t;

/* Transmit Function Pointer for Multi-Channel Output */
typedef u8 (*pfTransmitData)(u8 *pData, u32 length);

/* Command Definition Table Entry */
struct commandList_s {
    const char  *pCmd;                      /* Command String identifier (e.g. "SERVO", "MOVE_ABS") */
    u8          (*pFunction)(void *pVoid);  /* Command Handler Function */
    u32         option;                     /* Optional parameter passed to handler */
    u32         mode;                       /* Execution Mode Flags */
    const char  *pMessage;                  /* Response Header Prefix (e.g. ">>MOTOR", ";MOVE_ABS") */
    const char  *pNote;                     /* Command Description / Help String */
};

/* Data Passed to Each Command Handler */
typedef struct {
    u32             srcType;                /* Source Interface (UART / ETHERNET) */
    commandList_t   *pCmdList;              /* Pointer to active command list table */
    char            **pArgv;                /* Array of argument string tokens */
    u32             argc;                   /* Argument count (excluding current cmd name) */
    u32             option;                 /* Option value defined in command entry */
    u32             retryCnt;               /* Execution call count */
    u32             deactivate;             /* Deactivate flag */
    u32             channel;                /* Reserved channel index */
    u32             position;               /* Reserved position index */
} cmdFunctionData_t;

/* Global Command Engine State Handle */
typedef struct {
    u32             parseMaxCount;          /* Max tokens to parse */
    u32             receiveFlag;            /* 1 if new RX data is ready to process */
    u32             rxSrcType;              /* Current RX source */
    u32             rxDataSize;             /* RX Buffer Capacity */
    u8              *pRxData;               /* Pointer to RX Buffer */
    u32             rxDataCnt;              /* Current received bytes count */

    u32             txSrcType;              /* Destination TX source */
    u32             txDataSize;             /* TX Buffer Capacity */
    u8              *pTxData;               /* Pointer to TX Buffer */
    u32             txDataCnt;              /* Current response bytes count */

    const commandList_t *pCmdList;          /* Root command list table */

    u8              (*RunningState)(u8);    /* Optional callback when command is running */
    u8              (*ErrorState)(u8);      /* Optional callback on error */

    pfTransmitData  TransmitData[CMDSET_COMM_SRC_COUNT]; /* Channel transmit callbacks */
} cmdSetHandle_t;

extern cmdSetHandle_t cmdSetHandle;

#ifdef __cplusplus
}
#endif

#endif /* _COMMAND_SET_H_ */
