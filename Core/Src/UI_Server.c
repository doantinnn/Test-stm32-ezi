/**
  ******************************************************************************
  * @file    UI_Server.c
  * @brief   PC UI Communication Server Implementation
  *          Bridges Ethernet UDP (Port 5000) & RS-232 UART to Command Engine
  *          Referenced from TOP Engineering PG_StudyProject_Test
  ******************************************************************************
  */

#include "UI_Server.h"
#include "CmdSet_Process.h"
#include "CmdSet_Function.h"
#include "Cmd_Motor.h"
#include "Cmd_System.h"
#include "Cmd_IO.h"
#include "IO_Board.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include <stdio.h>
#include <string.h>

/* Static Buffers */
static u8 s_cmdRxBuffer[CMDSET_RX_BUF_SIZE];
static u8 s_cmdTxBuffer[CMDSET_TX_BUF_SIZE];

/* UART Line Reception Buffer */
static char     s_uart_rx_line[CMDSET_RX_BUF_SIZE];
static uint16_t s_uart_rx_idx = 0;

/* LwIP UDP PCB & Client Info for UI Communication */
static struct udp_pcb *s_ui_udp_pcb = NULL;
static ip_addr_t       s_pc_ip;
static u16_t           s_pc_port = 0;

/* Static Function Prototypes */
static u8 UI_UART_Transmit(u8 *pData, u32 length);
static u8 UI_Ethernet_Transmit(u8 *pData, u32 length);
static void UI_UDP_ReceiveCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void UI_UART_Process(void);

/* Direct Shortcut Command Handlers */
static u8 UI_ExecuteSubCommand(void *pVoid, const commandList_t *pList, const char *subCmdName)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    for (u32 i = 0; pList[i].pCmd != NULL; i++)
    {
        if (CMD_Compare(subCmdName, pList[i].pCmd))
        {
            if (pList[i].pFunction != NULL)
            {
                return pList[i].pFunction(pData);
            }
            break;
        }
    }
    CMD_Printf(";NO_SUBCMD=%s", subCmdName);
    return 0;
}

static u8 Cmd_Direct_Servo(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "servo");
}

static u8 Cmd_Direct_Mode(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "mode");
}

static u8 Cmd_Direct_Start(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "start");
}

static u8 Cmd_Direct_Jog(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "jog");
}

static u8 Cmd_Direct_Auto(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "auto_cycle");
}

static u8 Cmd_Direct_GetStatus(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "get_status");
}

static u8 Cmd_Direct_GetPos(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "get_pos");
}

static u8 Cmd_Direct_Stop(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "stop");
}

static u8 Cmd_Direct_EStop(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "estop");
}

static u8 Cmd_Direct_AlarmReset(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "alarm_reset");
}

static u8 Cmd_Direct_ClearPos(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "clear_pos");
}

static u8 Cmd_Direct_Move(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_Motor, "move_abs");
}

static u8 Cmd_Direct_Ping(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_System, "ping");
}

static u8 Cmd_Direct_Reset(void *pVoid)
{
    return UI_ExecuteSubCommand(pVoid, cmdList_System, "reset");
}

/* ==============================================================================
 * ROOT COMMAND TABLE
 * ============================================================================== */
const commandList_t cmdList_Root[] = {
//  { Command,          Function,               Option, Mode,           RootMessage,    Note }
    { "help",           Cmd_Help,               0,      CMD_MODE_NONE,  ">>HELP",       "Display available top-level commands: HELP [cmd]" },
    { "motor",          Cmd_Motor,              0,      CMD_MODE_NONE,  ">>MOTOR",      "Ezi-SERVO motor commands: MOTOR.<CMD> [ARGS...]" },
    { "system",         Cmd_System,             0,      CMD_MODE_NONE,  ">>SYSTEM",     "System control commands: SYSTEM.<CMD> [ARGS...]" },
    { "sys",            Cmd_System,             0,      CMD_MODE_NONE,  ">>SYS",        "System control commands: SYS.<CMD> [ARGS...]" },
    { "io",             Cmd_IO,                 0,      CMD_MODE_NONE,  ">>IO",         "IO extension commands: IO.<CMD> [ARGS...]" },

    /* Direct Terminal & UI Shortcuts */
    { "servo",          Cmd_Direct_Servo,       0,      CMD_MODE_NONE,  ">>MOTOR;SERVO","Shortcut to Servo ON/OFF: SERVO <0|1>" },
    { "mode",           Cmd_Direct_Mode,        0,      CMD_MODE_NONE,  ">>MOTOR;MODE", "Shortcut to Mode: MODE [0=MANUAL|1=AUTO|TOGGLE]" },
    { "start",          Cmd_Direct_Start,       0,      CMD_MODE_NONE,  ">>MOTOR;START","Shortcut to Start Auto Cycle: START [1=SINGLE|2=CONT]" },
    { "stop",           Cmd_Direct_Stop,        0,      CMD_MODE_NONE,  ">>MOTOR;STOP", "Shortcut to Stop Motion & Cycle: STOP" },
    { "jog",            Cmd_Direct_Jog,         0,      CMD_MODE_NONE,  ">>MOTOR;JOG",  "Shortcut to Jog motion: JOG <axis:X|Y|Z> <+|-|STOP> [speed]" },
    { "move",           Cmd_Direct_Move,        0,      CMD_MODE_NONE,  ">>MOTOR;MOVE_ABS","Shortcut to Move Abs: MOVE <axis:X|Y|Z> <pos> [speed]" },
    { "auto",           Cmd_Direct_Auto,        0,      CMD_MODE_NONE,  ">>MOTOR;AUTO", "Shortcut to Auto Cycle: AUTO <0=STOP|1=SINGLE|2=CONTINUOUS>" },
    { "status",         Cmd_Direct_GetStatus,   0,      CMD_MODE_NONE,  ">>MOTOR;STATUS","Shortcut to read motor status: STATUS" },
    { "pos",            Cmd_Direct_GetPos,      0,      CMD_MODE_NONE,  ">>MOTOR;POS",  "Shortcut to read motor position: POS" },
    { "estop",          Cmd_Direct_EStop,       0,      CMD_MODE_NONE,  ">>MOTOR;ESTOP","Shortcut to emergency stop motor: ESTOP" },
    { "alarm_reset",    Cmd_Direct_AlarmReset,  0,      CMD_MODE_NONE,  ">>MOTOR;ALARM_RESET", "Shortcut to reset driver alarm: ALARM_RESET" },
    { "clear_pos",      Cmd_Direct_ClearPos,    0,      CMD_MODE_NONE,  ">>MOTOR;CLEAR_POS", "Shortcut to clear pos (origin): CLEAR_POS" },
    { "ping",           Cmd_Direct_Ping,        0,      CMD_MODE_NONE,  ">>SYS;PING",   "Shortcut to ping test: PING" },
    { "reset",          Cmd_Direct_Reset,       0,      CMD_MODE_NONE,  ">>SYS;RESET",  "Shortcut to reset board: RESET" },

    { NULL,             NULL,                   0,      0,              NULL,           NULL }
};

/**
  * @brief  Initialize Command Engine and Communication Interfaces
  */
void UI_Server_Init(void)
{
    cmdSetHandle_t handle;
    memset(&handle, 0, sizeof(handle));

    handle.parseMaxCount = CMDSET_MAX_COUNT;
    handle.rxSrcType     = 0;
    handle.rxDataSize    = CMDSET_RX_BUF_SIZE;
    handle.pRxData       = s_cmdRxBuffer;
    handle.rxDataCnt     = 0;

    handle.txSrcType     = 0;
    handle.txDataSize    = CMDSET_TX_BUF_SIZE;
    handle.pTxData       = s_cmdTxBuffer;
    handle.txDataCnt     = 0;

    handle.pCmdList      = cmdList_Root;

    handle.TransmitData[CMD_COMM_SRC_TYPE_UART]     = UI_UART_Transmit;
    handle.TransmitData[CMD_COMM_SRC_TYPE_ETHERNET] = UI_Ethernet_Transmit;
    handle.TransmitData[CMD_COMM_SRC_TYPE_INTERNAL] = NULL;

    CMD_RegisterInterface(&handle);

    /* Setup LwIP UDP PCB for PC UI Application */
    s_ui_udp_pcb = udp_new();
    if (s_ui_udp_pcb != NULL)
    {
        err_t err = udp_bind(s_ui_udp_pcb, IP_ADDR_ANY, UI_SERVER_UDP_PORT);
        if (err == ERR_OK)
        {
            udp_recv(s_ui_udp_pcb, UI_UDP_ReceiveCallback, NULL);
            printf("[UI SERVER] PC UI Ethernet UDP listening on port %d\r\n", UI_SERVER_UDP_PORT);
        }
        else
        {
            printf("[UI SERVER] UDP bind error on port %d (err: %d)\r\n", UI_SERVER_UDP_PORT, err);
        }
    }

    printf("[UI SERVER] RS-232 UART Command Interface active at 115200 baud\r\n");
    printf("[UI SERVER] Command Engine ready. Type 'HELP' or 'MOTOR.HELP' to explore.\r\n");
}

/**
  * @brief  Periodic background polling for UI Communication (called in main loop)
  */
void UI_Server_Process(void)
{
    UI_UART_Process();
}

/**
  * @brief  RS-232 UART Character Receiver & Line Assembler
  */
static void UI_UART_Process(void)
{
    uint8_t ch;

    while (IOBoard_UART_GetChar(&ch))
    {
        if (ch == '\r' || ch == '\n')
        {
            if (s_uart_rx_idx > 0)
            {
                s_uart_rx_line[s_uart_rx_idx] = '\0';
                CMD_PutReceiveData(CMD_COMM_SRC_TYPE_UART, (const u8 *)s_uart_rx_line, s_uart_rx_idx);
                s_uart_rx_idx = 0;
                break;
            }
        }
        else if (ch == '\b' || ch == 0x7F)
        {
            if (s_uart_rx_idx > 0)
            {
                s_uart_rx_idx--;
            }
        }
        else
        {
            if (s_uart_rx_idx < (CMDSET_RX_BUF_SIZE - 1))
            {
                s_uart_rx_line[s_uart_rx_idx++] = (char)ch;
            }
        }
    }
}

/**
  * @brief  Transmit response packet over RS-232 UART
  */
static u8 UI_UART_Transmit(u8 *pData, u32 length)
{
    IOBoard_UART_TransmitBytes(pData, (uint16_t)length);
    return 1;
}

/**
  * @brief  Receive callback for incoming PC UI UDP packets
  */
static void UI_UDP_ReceiveCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)arg;
    (void)upcb;

    if (p == NULL) return;

    /* Remember remote PC IP and Port for returning response */
    s_pc_ip   = *addr;
    s_pc_port = port;

    if (p->tot_len < CMDSET_RX_BUF_SIZE)
    {
        char rx_buf[CMDSET_RX_BUF_SIZE];
        pbuf_copy_partial(p, rx_buf, p->tot_len, 0);
        rx_buf[p->tot_len] = '\0';

        CMD_PutReceiveData(CMD_COMM_SRC_TYPE_ETHERNET, (const u8 *)rx_buf, p->tot_len);
    }

    pbuf_free(p);
}

/**
  * @brief  Transmit response packet over Ethernet UDP back to PC
  */
static u8 UI_Ethernet_Transmit(u8 *pData, u32 length)
{
    if (s_ui_udp_pcb == NULL || length == 0) return 0;
    if (s_pc_port == 0) return 0;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)length, PBUF_RAM);
    if (p == NULL) return 0;

    memcpy(p->payload, pData, length);
    err_t err = udp_sendto(s_ui_udp_pcb, p, &s_pc_ip, s_pc_port);
    pbuf_free(p);

    return (err == ERR_OK) ? 1 : 0;
}
