/**
  ******************************************************************************
  * @file    Cmd_Motor.h
  * @brief   Ezi-SERVO Motor Motion Command Handlers and Command List
  *          Follows TOP Engineering PG_StudyProject_Test architecture
  ******************************************************************************
  */

#ifndef _CMD_MOTOR_H_
#define _CMD_MOTOR_H_

#include "CommandSet.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const commandList_t cmdList_Motor[];

u8 Cmd_Motor(void *pVoid);

#ifdef __cplusplus
}
#endif

#endif /* _CMD_MOTOR_H_ */
