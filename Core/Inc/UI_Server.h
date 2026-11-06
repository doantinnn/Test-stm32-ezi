/**
  ******************************************************************************
  * @file    UI_Server.h
  * @brief   PC UI Communication Server (Ethernet UDP Port 5000 + RS-232 UART)
  *          Dispatches commands to the TOP Engineering Command Engine
  ******************************************************************************
  */

#ifndef _UI_SERVER_H_
#define _UI_SERVER_H_

#include "CommandSet.h"
#include "eziservo_config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const commandList_t cmdList_Root[];

void UI_Server_Init(void);
void UI_Server_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* _UI_SERVER_H_ */
