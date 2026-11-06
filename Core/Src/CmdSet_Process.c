/**
  ******************************************************************************
  * @file    CmdSet_Process.c
  * @brief   Command Engine Registration & Main Processing Routine
  *          Referenced from TOP Engineering PG_StudyProject_Test
  ******************************************************************************
  */

#include "CmdSet_Process.h"
#include "CmdSet_Function.h"

cmdSetHandle_t cmdSetHandle;

extern u8 CMD_Analysis(void);

u8 CMD_RegisterInterface(const cmdSetHandle_t *pHandle)
{
    if (pHandle == NULL) return 0;

    memcpy(&cmdSetHandle, pHandle, sizeof(cmdSetHandle));
    return 1;
}

u8 CMD_PutReceiveData(u32 src, const u8 *pData, u32 length)
{
    if (src >= CMDSET_COMM_SRC_COUNT) return 0;
    if (pData == NULL || length == 0) return 0;
    if (length >= cmdSetHandle.rxDataSize) return 0;

    memcpy(cmdSetHandle.pRxData, pData, length);
    cmdSetHandle.rxDataCnt = length;
    cmdSetHandle.pRxData[length] = '\0';
    cmdSetHandle.rxSrcType = src;
    cmdSetHandle.receiveFlag = 1;

    return 1;
}

u8 CMD_ReceiveCheck(void)
{
    return (u8)cmdSetHandle.receiveFlag;
}

u8 CMD_ReceiveDataClear(void)
{
    cmdSetHandle.rxSrcType = 0;
    cmdSetHandle.rxDataCnt = 0;
    cmdSetHandle.receiveFlag = 0;
    return 1;
}

u8 CMD_TransmitDataClear(void)
{
    cmdSetHandle.txSrcType = cmdSetHandle.rxSrcType;
    cmdSetHandle.txDataCnt = 0;
    if (cmdSetHandle.pTxData != NULL && cmdSetHandle.txDataSize > 0)
    {
        cmdSetHandle.pTxData[0] = '\0';
    }
    return 1;
}

u8 CMD_Process(void)
{
    u8 result = 0;
    u8 chkr;

    chkr = CMD_ReceiveCheck();
    if (chkr == 0) return 0;

    CMD_RunningState(1);
    CMD_ErrorState(0);

    chkr = CMD_Analysis();
    if (chkr == 0)
    {
        CMD_TransmitDataClear();
        CMD_Printf(">>NO_COMMAND;ERROR\r\n");
        CMD_TransmitData();
        goto CMD_PROCESS_EXIT;
    }

    CMD_TransmitDataClear();

    chkr = CMD_Execute();
    if (chkr == 0)
    {
        CMD_ErrorState(1);
    }

    if (CMD_TransmitData() != 0)
    {
        result = 1;
    }

    CMD_RunningState(0);

CMD_PROCESS_EXIT:
    CMD_ReceiveDataClear();

    return result;
}
