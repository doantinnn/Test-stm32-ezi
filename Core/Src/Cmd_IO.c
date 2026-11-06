/**
  ******************************************************************************
  * @file    Cmd_IO.c
  * @brief   IO Extension Board Digital IO Command Implementation
  *          Follows TOP Engineering PG_StudyProject_Test architecture
  ******************************************************************************
  */

#include "Cmd_IO.h"
#include "CmdSet_Function.h"
#include "IO_Board.h"

extern volatile uint32_t g_btn_active_level;
volatile uint32_t g_io_manual_override = 0; // 0 = Auto Diagnostic LEDs, 1 = Manual User Control via UI

/* Static Handlers Prototypes */
static u8 Cmd_IO_GetInput(void *pVoid);
static u8 Cmd_IO_SetOutput(void *pVoid);
static u8 Cmd_IO_GetOutput(void *pVoid);
static u8 Cmd_IO_SetActiveLevel(void *pVoid);
static u8 Cmd_IO_SetAuto(void *pVoid);

/* IO Sub-command Table */
const commandList_t cmdList_IO[] = {
//  { Command,              Function,               Option, Mode,           RootMessage,        Note }
    { "help",               Cmd_Help,               0,      CMD_MODE_NONE,  ";HELP",            "Display IO command list: IO.HELP [cmd]" },
    { "get_input",          Cmd_IO_GetInput,        0,      CMD_MODE_NONE,  ";GET_INPUT",       "Read inputs J10/J11: IO.GET_INPUT" },
    { "set_output",         Cmd_IO_SetOutput,       0,      CMD_MODE_NONE,  ";SET_OUTPUT",      "Set LED output: IO.SET_OUTPUT <ch:0..3> <0=OFF|1=ON>" },
    { "get_output",         Cmd_IO_GetOutput,       0,      CMD_MODE_NONE,  ";GET_OUTPUT",      "Read all LED outputs: IO.GET_OUTPUT" },
    { "set_active_level",   Cmd_IO_SetActiveLevel,  0,      CMD_MODE_NONE,  ";ACTIVE_LEVEL",    "Set input logic level: IO.SET_ACTIVE_LEVEL <0=Active LOW|1=Active HIGH>" },
    { "set_auto",           Cmd_IO_SetAuto,         0,      CMD_MODE_NONE,  ";SET_AUTO",        "Auto Diagnostic LED Mode: IO.SET_AUTO <1=Auto|0=Manual>" },
    { NULL,                 NULL,                   0,      0,              NULL,               NULL }
};

/**
  * @brief  Top-level dispatcher for IO commands
  */
u8 Cmd_IO(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData == NULL || pData->argc == 0)
    {
        CMD_Printf(";ERR_USAGE=Usage: IO.<COMMAND> [ARGS...]. Type 'IO.HELP' for list");
        return 0;
    }

    return CMD_SubCommandExecute(pVoid, cmdList_IO);
}

static u8 Cmd_IO_GetInput(void *pVoid)
{
    (void)pVoid;

    u32 pos_pressed = IOBoard_GetJogPosPressed();
    u32 neg_pressed = IOBoard_GetJogNegPressed();
    u32 pos_raw     = IOBoard_GetJogPosRaw();
    u32 neg_raw     = IOBoard_GetJogNegRaw();

    CMD_Printf(";J10_JOG_POS=%lu;J11_JOG_NEG=%lu;RAW_POS=%lu;RAW_NEG=%lu;ACTIVE_LEVEL=%s",
               (unsigned long)pos_pressed,
               (unsigned long)neg_pressed,
               (unsigned long)pos_raw,
               (unsigned long)neg_raw,
               (g_btn_active_level == 0) ? "LOW" : "HIGH");

    return 1;
}

static u8 Cmd_IO_SetOutput(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    u32 channel;
    u32 state;

    if (pData->argc < 2)
    {
        CMD_Printf(";ERR_USAGE=Usage: IO.SET_OUTPUT <ch:0..3> <0=OFF|1=ON>");
        return 0;
    }

    channel = CMD_StrToUL(pData->pArgv[0]);
    state   = (CMD_StrToUL(pData->pArgv[1]) != 0) ? 1 : 0;

    /* Enable manual override so main loop diagnostic logic won't overwrite user outputs */
    g_io_manual_override = 1;

    switch (channel)
    {
        case 0:
            IOBoard_SetOutput_J100(state != 0);
            break;
        case 1:
            IOBoard_SetOutput_J101(state != 0);
            break;
        case 2:
            IOBoard_SetOutput_J102(state != 0);
            break;
        case 3:
            IOBoard_SetOutput_J103(state != 0);
            break;
        default:
            CMD_Printf(";ERR_PARAM=Channel must be 0..3 (J100..J103)");
            return 0;
    }

    CMD_Printf(";CH=%lu;STATE=%s;OVERRIDE=MANUAL", (unsigned long)channel, (state != 0) ? "ON" : "OFF");
    return 1;
}

static u8 Cmd_IO_GetOutput(void *pVoid)
{
    (void)pVoid;

    CMD_Printf(";J100=%d;J101=%d;J102=%d;J103=%d;AUTO_DIAG=%s",
               IOBoard_GetOutput(0) ? 1 : 0,
               IOBoard_GetOutput(1) ? 1 : 0,
               IOBoard_GetOutput(2) ? 1 : 0,
               IOBoard_GetOutput(3) ? 1 : 0,
               (g_io_manual_override == 0) ? "ON" : "OFF");

    return 1;
}

static u8 Cmd_IO_SetActiveLevel(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData->argc < 1)
    {
        CMD_Printf(";ERR_USAGE=Usage: IO.SET_ACTIVE_LEVEL <0=Active LOW|1=Active HIGH>");
        return 0;
    }

    g_btn_active_level = (CMD_StrToUL(pData->pArgv[0]) != 0) ? 1 : 0;
    CMD_Printf(";ACTIVE_LEVEL=%s", (g_btn_active_level == 0) ? "LOW" : "HIGH");
    return 1;
}

static u8 Cmd_IO_SetAuto(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData->argc < 1)
    {
        CMD_Printf(";AUTO_DIAG=%s", (g_io_manual_override == 0) ? "ON" : "OFF");
        return 1;
    }

    u32 enable = CMD_StrToUL(pData->pArgv[0]);
    g_io_manual_override = (enable != 0) ? 0 : 1;
    CMD_Printf(";AUTO_DIAG=%s", (g_io_manual_override == 0) ? "ON" : "OFF");
    return 1;
}
