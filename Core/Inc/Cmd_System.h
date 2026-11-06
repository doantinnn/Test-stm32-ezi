/**
  ******************************************************************************
  * @file    Cmd_System.h
  * @brief   System-level Command Handlers and Command List
  *          Follows TOP Engineering PG_StudyProject_Test architecture
  ******************************************************************************
  */

#ifndef _CMD_SYSTEM_H_
#define _CMD_SYSTEM_H_

#include "CommandSet.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const commandList_t cmdList_System[];

u8 Cmd_System(void *pVoid);

#ifdef __cplusplus
}
#endif

#endif /* _CMD_SYSTEM_H_ */
