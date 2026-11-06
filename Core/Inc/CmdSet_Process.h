/**
  ******************************************************************************
  * @file    CmdSet_Process.h
  * @brief   Command Engine Registration & Processing Interface Prototypes
  *          Referenced from TOP Engineering PG_StudyProject_Test
  ******************************************************************************
  */

#ifndef _CMDSET_PROCESS_H_
#define _CMDSET_PROCESS_H_

#include "CommandSet.h"

#ifdef __cplusplus
extern "C" {
#endif

u8 CMD_RegisterInterface(const cmdSetHandle_t *pHandle);
u8 CMD_PutReceiveData(u32 src, const u8 *pData, u32 length);
u8 CMD_ReceiveCheck(void);
u8 CMD_Process(void);
u8 CMD_ReceiveDataClear(void);
u8 CMD_TransmitDataClear(void);

#ifdef __cplusplus
}
#endif

#endif /* _CMDSET_PROCESS_H_ */
