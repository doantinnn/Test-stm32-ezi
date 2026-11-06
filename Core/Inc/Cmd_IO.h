/**
  ******************************************************************************
  * @file    Cmd_IO.h
  * @brief   IO Extension Board Digital IO Command Handlers and Command List
  *          Follows TOP Engineering PG_StudyProject_Test architecture
  ******************************************************************************
  */

#ifndef _CMD_IO_H_
#define _CMD_IO_H_

#include "CommandSet.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const commandList_t cmdList_IO[];

u8 Cmd_IO(void *pVoid);

#ifdef __cplusplus
}
#endif

#endif /* _CMD_IO_H_ */
