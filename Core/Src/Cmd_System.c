/**
  ******************************************************************************
  * @file    Cmd_System.c
  * @brief   System-level Command Implementation
  *          Follows TOP Engineering PG_StudyProject_Test architecture
  ******************************************************************************
  */

#include "Cmd_System.h"
#include "CmdSet_Function.h"
#include "IO_Board.h"
#include "eziservo_config.h"

/* Static Handlers Prototypes */
static u8 Cmd_System_Info(void *pVoid);
static u8 Cmd_System_Ping(void *pVoid);
static u8 Cmd_System_Reset(void *pVoid);

/* System Sub-command Table */
const commandList_t cmdList_System[] = {
//  { Command,          Function,               Option, Mode,           RootMessage,    Note }
    { "help",           Cmd_Help,               0,      CMD_MODE_NONE,  ";HELP",        "Display system command list: SYS.HELP [cmd]" },
    { "info",           Cmd_System_Info,        0,      CMD_MODE_NONE,  ";INFO",        "Get system information and configuration: SYS.INFO" },
    { "ping",           Cmd_System_Ping,        0,      CMD_MODE_NONE,  ";PING",        "Ping communication test: SYS.PING" },
    { "reset",          Cmd_System_Reset,       0,      CMD_MODE_NONE,  ";RESET",       "System software reboot: SYS.RESET" },
    { NULL,             NULL,                   0,      0,              NULL,           NULL }
};

/**
  * @brief  Top-level dispatcher for SYSTEM / SYS commands
  */
u8 Cmd_System(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData == NULL || pData->argc == 0)
    {
        CMD_Printf(";ERR_USAGE=Usage: SYS.<COMMAND> [ARGS...]. Type 'SYS.HELP' for list");
        return 0;
    }

    return CMD_SubCommandExecute(pVoid, cmdList_System);
}

static u8 Cmd_System_Info(void *pVoid)
{
    (void)pVoid;

    CMD_Printf(";BOARD=E7502_IO_EXTENSION;ID=%d;MCU=STM32F767ZI;CLK=216MHz;FW=1.0.0;IP=%d.%d.%d.%d;DRIVER_IP=%s:%d;UPTIME_MS=%lu",
               IOBoard_GetBoardID(),
               sysConfig.ipAdrs[0], sysConfig.ipAdrs[1], sysConfig.ipAdrs[2], sysConfig.ipAdrs[3],
               DRIVER_IP_ADDR, DRIVER_UDP_PORT,
               (unsigned long)HAL_GetTick());

    return 1;
}

static u8 Cmd_System_Ping(void *pVoid)
{
    (void)pVoid;

    CMD_Printf(";PONG;TICK=%lu", (unsigned long)HAL_GetTick());
    return 1;
}

static u8 Cmd_System_Reset(void *pVoid)
{
    (void)pVoid;

    CMD_Printf(";REBOOTING");
    CMD_TransmitData();

    HAL_Delay(100);
    NVIC_SystemReset();

    return 1;
}
