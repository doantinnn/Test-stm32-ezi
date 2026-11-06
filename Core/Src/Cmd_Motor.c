/**
  ******************************************************************************
  * @file    Cmd_Motor.c
  * @brief   3-Axis (X, Y, Z) Ezi-SERVO Motor Motion Command Implementation
  *          Follows TOP Engineering PG_StudyProject_Test architecture
  ******************************************************************************
  */

#include "Cmd_Motor.h"
#include "CmdSet_Function.h"
#include "ezi_servo_driver.h"
#include "main.h"

/* Helper function: Parse Axis token (0/X, 1/Y, 2/Z, ALL/3) */
static int Parse_Axis(const char *pStr, int default_axis)
{
    if (pStr == NULL) return default_axis;
    if (CMD_Compare(pStr, "0") || CMD_Compare(pStr, "X") || CMD_Compare(pStr, "AXIS_X")) return AXIS_X;
    if (CMD_Compare(pStr, "1") || CMD_Compare(pStr, "Y") || CMD_Compare(pStr, "AXIS_Y")) return AXIS_Y;
    if (CMD_Compare(pStr, "2") || CMD_Compare(pStr, "Z") || CMD_Compare(pStr, "AXIS_Z")) return AXIS_Z;
    if (CMD_Compare(pStr, "ALL") || CMD_Compare(pStr, "3") || CMD_Compare(pStr, "*")) return AXIS_ALL;
    return -1;
}

static const char *Get_AxisName(int axis)
{
    switch (axis) {
        case AXIS_X: return "X";
        case AXIS_Y: return "Y";
        case AXIS_Z: return "Z";
        case AXIS_ALL: return "ALL";
        default: return "?";
    }
}

/* Static Handlers Prototypes */
static u8 Cmd_Motor_Servo(void *pVoid);
static u8 Cmd_Motor_AlarmReset(void *pVoid);
static u8 Cmd_Motor_ClearPos(void *pVoid);
static u8 Cmd_Motor_MoveAbs(void *pVoid);
static u8 Cmd_Motor_MoveInc(void *pVoid);
static u8 Cmd_Motor_MoveVel(void *pVoid);
static u8 Cmd_Motor_Start(void *pVoid);
static u8 Cmd_Motor_Stop(void *pVoid);
static u8 Cmd_Motor_EStop(void *pVoid);
static u8 Cmd_Motor_GetPos(void *pVoid);
static u8 Cmd_Motor_GetStatus(void *pVoid);
static u8 Cmd_Motor_Jog(void *pVoid);
static u8 Cmd_Motor_SetSpeed(void *pVoid);
static u8 Cmd_Motor_SetMode(void *pVoid);
static u8 Cmd_Motor_GetMode(void *pVoid);
static u8 Cmd_Motor_AutoCycle(void *pVoid);
static u8 Cmd_Motor_AutoConfig(void *pVoid);

/* Motor Sub-command Table */
const commandList_t cmdList_Motor[] = {
//  { Command,          Function,               Option, Mode,           RootMessage,        Note }
    { "help",           Cmd_Help,               0,      CMD_MODE_NONE,  ";HELP",            "Display motor command list: MOTOR.HELP [cmd]" },
    { "servo",          Cmd_Motor_Servo,        0,      CMD_MODE_NONE,  ";SERVO",           "Servo ON/OFF: MOTOR.SERVO <axis:X|Y|Z|ALL> <0|1>" },
    { "enable",         Cmd_Motor_Servo,        0,      CMD_MODE_NONE,  ";ENABLE",          "Servo Enable: MOTOR.ENABLE <axis:X|Y|Z|ALL> <0|1>" },
    { "alarm_reset",    Cmd_Motor_AlarmReset,   0,      CMD_MODE_NONE,  ";ALARM_RESET",     "Reset driver alarm: MOTOR.ALARM_RESET [axis:X|Y|Z|ALL]" },
    { "clear_pos",      Cmd_Motor_ClearPos,     0,      CMD_MODE_NONE,  ";CLEAR_POS",       "Set current pos as origin: MOTOR.CLEAR_POS [axis:X|Y|Z|ALL]" },
    { "start",          Cmd_Motor_Start,        0,      CMD_MODE_NONE,  ";START",           "Start 3-Axis Auto Cycle: MOTOR.START [1=SINGLE|2=CONTINUOUS]" },
    { "move_abs",       Cmd_Motor_MoveAbs,      0,      CMD_MODE_NONE,  ";MOVE_ABS",        "Absolute position move: MOTOR.MOVE_ABS <axis:X|Y|Z> <pos> [speed]" },
    { "move_inc",       Cmd_Motor_MoveInc,      0,      CMD_MODE_NONE,  ";MOVE_INC",        "Incremental position move: MOTOR.MOVE_INC <axis:X|Y|Z> <dist> [speed]" },
    { "move_vel",       Cmd_Motor_MoveVel,      0,      CMD_MODE_NONE,  ";MOVE_VEL",        "Velocity move: MOTOR.MOVE_VEL <axis:X|Y|Z> <speed> <dir:1=CW,0=CCW>" },
    { "stop",           Cmd_Motor_Stop,         0,      CMD_MODE_NONE,  ";STOP",            "Decelerated stop: MOTOR.STOP [axis:X|Y|Z|ALL]" },
    { "estop",          Cmd_Motor_EStop,        0,      CMD_MODE_NONE,  ";ESTOP",           "Emergency stop all axes: MOTOR.ESTOP" },
    { "get_pos",        Cmd_Motor_GetPos,       0,      CMD_MODE_NONE,  ";GET_POS",         "Read position counters: MOTOR.GET_POS [axis:X|Y|Z|ALL]" },
    { "pos",            Cmd_Motor_GetPos,       0,      CMD_MODE_NONE,  ";POS",             "Read position counters: MOTOR.POS [axis:X|Y|Z|ALL]" },
    { "get_status",     Cmd_Motor_GetStatus,    0,      CMD_MODE_NONE,  ";GET_STATUS",      "Read comprehensive status: MOTOR.GET_STATUS [axis:X|Y|Z|ALL]" },
    { "status",         Cmd_Motor_GetStatus,    0,      CMD_MODE_NONE,  ";STATUS",          "Read status: MOTOR.STATUS [axis:X|Y|Z|ALL]" },
    { "jog",            Cmd_Motor_Jog,          0,      CMD_MODE_NONE,  ";JOG",             "Jog motion: MOTOR.JOG <axis:X|Y|Z> <+|-|STOP> [speed]" },
    { "set_speed",      Cmd_Motor_SetSpeed,     0,      CMD_MODE_NONE,  ";SET_SPEED",       "Set speed (pps): MOTOR.SET_SPEED <axis:X|Y|Z|ALL> <speed>" },
    { "set_mode",       Cmd_Motor_SetMode,      0,      CMD_MODE_NONE,  ";SET_MODE",        "Set operation mode: MOTOR.SET_MODE <0=MANUAL|1=AUTO>" },
    { "mode",           Cmd_Motor_SetMode,      0,      CMD_MODE_NONE,  ";MODE",            "Get/Set operation mode: MOTOR.MODE [0=MANUAL|1=AUTO|TOGGLE]" },
    { "get_mode",       Cmd_Motor_GetMode,      0,      CMD_MODE_NONE,  ";GET_MODE",        "Get operation mode: MOTOR.GET_MODE" },
    { "auto_cycle",     Cmd_Motor_AutoCycle,    0,      CMD_MODE_NONE,  ";AUTO_CYCLE",      "Control auto cycle: MOTOR.AUTO_CYCLE <0=STOP|1=SINGLE|2=CONTINUOUS>" },
    { "auto_config",    Cmd_Motor_AutoConfig,   0,      CMD_MODE_NONE,  ";AUTO_CONFIG",     "Config 3-Axis cycle: MOTOR.AUTO_CONFIG <x_work> <y_disp> <z_out> [disp_ms]" },
    { NULL,             NULL,                   0,      0,              NULL,               NULL }
};

/**
  * @brief  Top-level dispatcher for MOTOR commands
  */
u8 Cmd_Motor(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData == NULL || pData->argc == 0)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.<COMMAND> [ARGS...]. Type 'MOTOR.HELP' for list");
        return 0;
    }

    return CMD_SubCommandExecute(pVoid, cmdList_Motor);
}

/**
  * @brief  Servo ON/OFF Command
  *         Syntax: MOTOR.SERVO <0|1>               -> Affects ALL 3 axes
  *                 MOTOR.SERVO <axis:X|Y|Z|ALL> <0|1>
  */
static u8 Cmd_Motor_Servo(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis = AXIS_ALL;
    u32 enable = 1;

    if (pData->argc < 1)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.SERVO <axis:X|Y|Z|ALL> <0=OFF|1=ON> or MOTOR.SERVO <0|1>");
        return 0;
    }

    if (pData->argc == 1)
    {
        /* 1 argument: apply to ALL axes */
        enable = CMD_StrToUL(pData->pArgv[0]);
        axis = AXIS_ALL;
    }
    else
    {
        /* 2 arguments: axis + enable */
        axis = Parse_Axis(pData->pArgv[0], -1);
        if (axis < 0)
        {
            CMD_Printf(";ERR_PARAM=Invalid axis '%s'. Use X, Y, Z, or ALL", pData->pArgv[0]);
            return 0;
        }
        enable = CMD_StrToUL(pData->pArgv[1]);
    }

    if (axis == AXIS_ALL)
    {
        for (int i = 0; i < NUM_AXES; i++)
        {
            if (g_ezi_servo[i].is_initialized)
            {
                EziServo_SetEnable(&g_ezi_servo[i], (enable != 0));
            }
        }
        CMD_Printf(";AXIS=ALL;STATE=%s", (enable != 0) ? "ON" : "OFF");
        return 1;
    }
    else
    {
        int res = EziServo_SetEnable(&g_ezi_servo[axis], (enable != 0));
        if (res == EZI_OK)
        {
            CMD_Printf(";AXIS=%s;STATE=%s", Get_AxisName(axis), (enable != 0) ? "ON" : "OFF");
            return 1;
        }
        CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
        return 0;
    }
}

/**
  * @brief  Alarm Reset Command
  *         Syntax: MOTOR.ALARM_RESET [axis:X|Y|Z|ALL]
  */
static u8 Cmd_Motor_AlarmReset(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis = AXIS_ALL;

    if (pData != NULL && pData->argc >= 1)
    {
        axis = Parse_Axis(pData->pArgv[0], AXIS_ALL);
    }

    if (axis == AXIS_ALL)
    {
        for (int i = 0; i < NUM_AXES; i++)
        {
            if (g_ezi_servo[i].is_initialized)
            {
                EziServo_AlarmReset(&g_ezi_servo[i]);
            }
        }
        CMD_Printf(";AXIS=ALL;ALARM_RESET=SUCCESS");
        return 1;
    }
    else if (axis >= 0 && axis < NUM_AXES)
    {
        int res = EziServo_AlarmReset(&g_ezi_servo[axis]);
        if (res == EZI_OK)
        {
            CMD_Printf(";AXIS=%s;ALARM_RESET=SUCCESS", Get_AxisName(axis));
            return 1;
        }
        CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
        return 0;
    }

    CMD_Printf(";ERR_PARAM=Invalid axis. Use X, Y, Z, or ALL");
    return 0;
}

/**
  * @brief  Clear Position Counter (Origin Set)
  *         Syntax: MOTOR.CLEAR_POS [axis:X|Y|Z|ALL]
  */
static u8 Cmd_Motor_ClearPos(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis = AXIS_ALL;

    if (pData != NULL && pData->argc >= 1)
    {
        axis = Parse_Axis(pData->pArgv[0], AXIS_ALL);
    }

    if (axis == AXIS_ALL)
    {
        for (int i = 0; i < NUM_AXES; i++)
        {
            if (g_ezi_servo[i].is_initialized)
            {
                EziServo_ClearPosition(&g_ezi_servo[i]);
            }
        }
        CMD_Printf(";AXIS=ALL;POS_CLEARED=0");
        return 1;
    }
    else if (axis >= 0 && axis < NUM_AXES)
    {
        int res = EziServo_ClearPosition(&g_ezi_servo[axis]);
        if (res == EZI_OK)
        {
            CMD_Printf(";AXIS=%s;POS_CLEARED=0", Get_AxisName(axis));
            return 1;
        }
        CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
        return 0;
    }

    CMD_Printf(";ERR_PARAM=Invalid axis. Use X, Y, Z, or ALL");
    return 0;
}

/**
  * @brief  Absolute Position Move Command
  *         Syntax: MOTOR.MOVE_ABS <axis:X|Y|Z> <target_pos> [speed_pps]
  */
static u8 Cmd_Motor_MoveAbs(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis;
    s32 target_pos;
    u32 speed;
    int res;

    if (g_operation_mode != 0)
    {
        CMD_Printf(";FAIL;ERR_MODE=Manual motion locked in AUTO mode. Switch to MANUAL first");
        return 0;
    }

    if (pData->argc < 2)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.MOVE_ABS <axis:X|Y|Z> <pos_pulse> [speed_pps]");
        return 0;
    }

    axis = Parse_Axis(pData->pArgv[0], -1);
    if (axis < 0 || axis >= NUM_AXES)
    {
        CMD_Printf(";ERR_PARAM=Invalid axis '%s'. Use X, Y, or Z", pData->pArgv[0]);
        return 0;
    }

    target_pos = CMD_StrToL(pData->pArgv[1]);
    speed = (pData->argc >= 3) ? CMD_StrToUL(pData->pArgv[2]) : g_jog_speed_pps[axis];
    if (speed == 0) speed = 10000;

    /* Ensure Servo is ON before motion */
    if (!(g_driver_axis_status[axis] & FLAG_SERVO_ON))
    {
        EziServo_SetEnable(&g_ezi_servo[axis], true);
    }

    res = EziServo_MoveAbs(&g_ezi_servo[axis], target_pos, speed);
    if (res == EZI_OK)
    {
        CMD_Printf(";AXIS=%s;TARGET=%ld;SPEED=%lu", Get_AxisName(axis), (long)target_pos, (unsigned long)speed);
        return 1;
    }

    CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
    return 0;
}

/**
  * @brief  Incremental Position Move Command
  *         Syntax: MOTOR.MOVE_INC <axis:X|Y|Z> <dist_pulse> [speed_pps]
  */
static u8 Cmd_Motor_MoveInc(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis;
    s32 dist;
    u32 speed;
    int res;

    if (g_operation_mode != 0)
    {
        CMD_Printf(";FAIL;ERR_MODE=Manual motion locked in AUTO mode. Switch to MANUAL first");
        return 0;
    }

    if (pData->argc < 2)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.MOVE_INC <axis:X|Y|Z> <dist_pulse> [speed_pps]");
        return 0;
    }

    axis = Parse_Axis(pData->pArgv[0], -1);
    if (axis < 0 || axis >= NUM_AXES)
    {
        CMD_Printf(";ERR_PARAM=Invalid axis '%s'. Use X, Y, or Z", pData->pArgv[0]);
        return 0;
    }

    dist = CMD_StrToL(pData->pArgv[1]);
    speed = (pData->argc >= 3) ? CMD_StrToUL(pData->pArgv[2]) : g_jog_speed_pps[axis];
    if (speed == 0) speed = 10000;

    if (!(g_driver_axis_status[axis] & FLAG_SERVO_ON))
    {
        EziServo_SetEnable(&g_ezi_servo[axis], true);
    }

    res = EziServo_MoveInc(&g_ezi_servo[axis], dist, speed);
    if (res == EZI_OK)
    {
        CMD_Printf(";AXIS=%s;DIST=%ld;SPEED=%lu", Get_AxisName(axis), (long)dist, (unsigned long)speed);
        return 1;
    }

    CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
    return 0;
}

/**
  * @brief  Continuous Velocity Move Command
  *         Syntax: MOTOR.MOVE_VEL <axis:X|Y|Z> <speed_pps> [dir:1=CW,0=CCW]
  */
static u8 Cmd_Motor_MoveVel(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis;
    u32 speed;
    u32 dir = 1;
    int res;

    if (g_operation_mode != 0)
    {
        CMD_Printf(";FAIL;ERR_MODE=Manual motion locked in AUTO mode. Switch to MANUAL first");
        return 0;
    }

    if (pData->argc < 2)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.MOVE_VEL <axis:X|Y|Z> <speed_pps> [dir:1=CW,0=CCW]");
        return 0;
    }

    axis = Parse_Axis(pData->pArgv[0], -1);
    if (axis < 0 || axis >= NUM_AXES)
    {
        CMD_Printf(";ERR_PARAM=Invalid axis '%s'. Use X, Y, or Z", pData->pArgv[0]);
        return 0;
    }

    speed = CMD_StrToUL(pData->pArgv[1]);
    if (pData->argc >= 3)
    {
        dir = (CMD_StrToUL(pData->pArgv[2]) != 0) ? 1 : 0;
    }

    if (!(g_driver_axis_status[axis] & FLAG_SERVO_ON))
    {
        EziServo_SetEnable(&g_ezi_servo[axis], true);
    }

    res = EziServo_MoveVelocity(&g_ezi_servo[axis], speed, dir);
    if (res == EZI_OK)
    {
        CMD_Printf(";AXIS=%s;SPEED=%lu;DIR=%s", Get_AxisName(axis), (unsigned long)speed, (dir != 0) ? "CW" : "CCW");
        return 1;
    }

    CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
    return 0;
}

/**
  * @brief  Start 3-Axis Auto Cycle
  *         Syntax: MOTOR.START [1=SINGLE|2=CONTINUOUS]
  */
static u8 Cmd_Motor_Start(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    u32 cycle_mode = 1;

    if (pData != NULL && pData->argc >= 1)
    {
        cycle_mode = CMD_StrToUL(pData->pArgv[0]);
        if (cycle_mode == 0) cycle_mode = 1;
    }

    /* Request switch to AUTO mode and start cycle */
    Request_OperationModeSwitch(OP_MODE_AUTO);
    g_auto_cycle_trigger = cycle_mode;
    g_auto_step = 0;

    CMD_Printf(";MODE=AUTO;CYCLE=%s;TRIGGER=%lu;STATUS=3AXIS_DISPENSING_STARTED", 
               (cycle_mode == 2) ? "CONTINUOUS" : "SINGLE", 
               (unsigned long)cycle_mode);
    return 1;
}

/**
  * @brief  Decelerated Stop Command
  *         Syntax: MOTOR.STOP [axis:X|Y|Z|ALL]
  */
static u8 Cmd_Motor_Stop(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis = AXIS_ALL;

    if (pData != NULL && pData->argc >= 1)
    {
        axis = Parse_Axis(pData->pArgv[0], AXIS_ALL);
    }

    /* Stop auto cycle and clear pending mode switch */
    g_auto_cycle_trigger = 0;
    g_auto_step = 0;
    g_mode_switch_pending = MODE_SWITCH_NONE;

    if (axis == AXIS_ALL)
    {
        for (int i = 0; i < NUM_AXES; i++)
        {
            g_jog_state[i] = JOG_STATE_IDLE;
            if (g_ezi_servo[i].is_initialized)
            {
                EziServo_MoveStop(&g_ezi_servo[i]);
            }
        }
        CMD_Printf(";AXIS=ALL;DECEL_STOP=SUCCESS;CYCLE=STOPPED");
        return 1;
    }
    else if (axis >= 0 && axis < NUM_AXES)
    {
        g_jog_state[axis] = JOG_STATE_IDLE;
        int res = EziServo_MoveStop(&g_ezi_servo[axis]);
        if (res == EZI_OK)
        {
            CMD_Printf(";AXIS=%s;DECEL_STOP=SUCCESS", Get_AxisName(axis));
            return 1;
        }
        CMD_Printf(";AXIS=%s;FAIL;ERR_CODE=%d", Get_AxisName(axis), res);
        return 0;
    }

    CMD_Printf(";ERR_PARAM=Invalid axis. Use X, Y, Z, or ALL");
    return 0;
}

/**
  * @brief  Immediate Emergency Stop Command on ALL 3 Axes
  *         Syntax: MOTOR.ESTOP
  */
static u8 Cmd_Motor_EStop(void *pVoid)
{
    (void)pVoid;

    g_auto_cycle_trigger = 0;
    g_auto_step = 0;
    g_mode_switch_pending = MODE_SWITCH_NONE;

    for (int i = 0; i < NUM_AXES; i++)
    {
        g_jog_state[i] = JOG_STATE_IDLE;
        if (g_ezi_servo[i].is_initialized)
        {
            EziServo_EmergencyStop(&g_ezi_servo[i]);
        }
    }

    CMD_Printf(";EMERGENCY_STOP=ALL_AXES_STOPPED;CYCLE=ABORTED");
    return 1;
}

/**
  * @brief  Read Position Counters
  *         Syntax: MOTOR.GET_POS [axis:X|Y|Z|ALL]
  */
static u8 Cmd_Motor_GetPos(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis = AXIS_ALL;

    if (pData != NULL && pData->argc >= 1)
    {
        axis = Parse_Axis(pData->pArgv[0], AXIS_ALL);
    }

    if (axis == AXIS_ALL)
    {
        CMD_Printf(";X_ACT=%ld;X_CMD=%ld;Y_ACT=%ld;Y_CMD=%ld;Z_ACT=%ld;Z_CMD=%ld",
                   (long)g_driver_act_pos[AXIS_X], (long)g_driver_cmd_pos[AXIS_X],
                   (long)g_driver_act_pos[AXIS_Y], (long)g_driver_cmd_pos[AXIS_Y],
                   (long)g_driver_act_pos[AXIS_Z], (long)g_driver_cmd_pos[AXIS_Z]);
        return 1;
    }
    else if (axis >= 0 && axis < NUM_AXES)
    {
        CMD_Printf(";AXIS=%s;ACT_POS=%ld;CMD_POS=%ld;ERR_POS=%ld",
                   Get_AxisName(axis),
                   (long)g_driver_act_pos[axis],
                   (long)g_driver_cmd_pos[axis],
                   (long)(g_driver_cmd_pos[axis] - g_driver_act_pos[axis]));
        return 1;
    }

    CMD_Printf(";ERR_PARAM=Invalid axis. Use X, Y, Z, or ALL");
    return 0;
}

/**
  * @brief  Read Comprehensive 3-Axis Status
  *         Syntax: MOTOR.GET_STATUS [axis:X|Y|Z|ALL]
  */
static u8 Cmd_Motor_GetStatus(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis = AXIS_ALL;

    if (pData != NULL && pData->argc >= 1)
    {
        axis = Parse_Axis(pData->pArgv[0], AXIS_ALL);
    }

    if (axis == AXIS_ALL)
    {
        CMD_Printf(";X_ACT=%ld;X_ST=0x%08X;Y_ACT=%ld;Y_ST=0x%08X;Z_ACT=%ld;Z_ST=0x%08X;MODE=%s;STEP=%lu;CYCLE=%lu",
                   (long)g_driver_act_pos[AXIS_X], (unsigned int)g_driver_axis_status[AXIS_X],
                   (long)g_driver_act_pos[AXIS_Y], (unsigned int)g_driver_axis_status[AXIS_Y],
                   (long)g_driver_act_pos[AXIS_Z], (unsigned int)g_driver_axis_status[AXIS_Z],
                   (g_operation_mode == 0) ? "MANUAL" : "AUTO",
                   (unsigned long)g_auto_step,
                   (unsigned long)g_auto_cycle_count);
        return 1;
    }
    else if (axis >= 0 && axis < NUM_AXES)
    {
        uint32_t st = g_driver_axis_status[axis];
        CMD_Printf(";AXIS=%s;SERVO=%d;MOVING=%d;INPOS=%d;ALARM=%d;ALM_CODE=%lu;AXIS_STATUS=0x%08X;ACT_POS=%ld;CMD_POS=%ld",
                   Get_AxisName(axis),
                   (st & FLAG_SERVO_ON) ? 1 : 0,
                   (st & FLAG_MOTION_MOVING) ? 1 : 0,
                   (st & FLAG_IN_POSITION) ? 1 : 0,
                   (st & (FLAG_DRIVE_FAULT | FLAG_ERROR_ALL)) ? 1 : 0,
                   (unsigned long)g_driver_alarm_code[axis],
                   (unsigned int)st,
                   (long)g_driver_act_pos[axis],
                   (long)g_driver_cmd_pos[axis]);
        return 1;
    }

    CMD_Printf(";ERR_PARAM=Invalid axis. Use X, Y, Z, or ALL");
    return 0;
}

/**
  * @brief  Manual Jog Motion Command
  *         Syntax: MOTOR.JOG <axis:X|Y|Z> <+|-|STOP> [speed_pps]
  *                 MOTOR.JOG STOP                -> Stops all axes
  */
static u8 Cmd_Motor_Jog(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis;
    const char *pDirStr;
    u32 speed;
    int res = EZI_OK;

    if (pData->argc < 1)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.JOG <axis:X|Y|Z> <+|-|STOP> [speed] or MOTOR.JOG STOP");
        return 0;
    }

    /* Shortcut: JOG STOP */
    if (CMD_Compare(pData->pArgv[0], "STOP") || CMD_Compare(pData->pArgv[0], "0"))
    {
        for (int i = 0; i < NUM_AXES; i++)
        {
            g_jog_state[i] = JOG_STATE_IDLE;
            if (g_ezi_servo[i].is_initialized) EziServo_MoveStop(&g_ezi_servo[i]);
        }
        CMD_Printf(";DIR=STOP;ALL_AXES=STOPPED");
        return 1;
    }

    if (pData->argc < 2)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.JOG <axis:X|Y|Z> <+|-|STOP> [speed]");
        return 0;
    }

    axis = Parse_Axis(pData->pArgv[0], -1);
    if (axis < 0 || axis >= NUM_AXES)
    {
        CMD_Printf(";ERR_PARAM=Invalid axis '%s'. Use X, Y, or Z", pData->pArgv[0]);
        return 0;
    }

    pDirStr = pData->pArgv[1];

    /* Lock manual Jog motion in AUTO mode (allow STOP only) */
    if (g_operation_mode != 0 && !CMD_Compare(pDirStr, "STOP") && !CMD_Compare(pDirStr, "0"))
    {
        CMD_Printf(";FAIL;ERR_MODE=Jog motion locked in AUTO mode. Switch to MANUAL first");
        return 0;
    }

    speed = (pData->argc >= 3) ? CMD_StrToUL(pData->pArgv[2]) : g_jog_speed_pps[axis];
    if (speed == 0) speed = 10000;

    if (CMD_Compare(pDirStr, "+") || CMD_Compare(pDirStr, "POS") || CMD_Compare(pDirStr, "1"))
    {
        if (!(g_driver_axis_status[axis] & FLAG_SERVO_ON))
        {
            EziServo_SetEnable(&g_ezi_servo[axis], true);
            HAL_Delay(10);
        }
        res = EziServo_MoveVelocity(&g_ezi_servo[axis], speed, 1);
        g_jog_state[axis] = JOG_STATE_FORWARD;
        CMD_Printf(";AXIS=%s;DIR=CW;SPEED=%lu", Get_AxisName(axis), (unsigned long)speed);
    }
    else if (CMD_Compare(pDirStr, "-") || CMD_Compare(pDirStr, "NEG") || CMD_Compare(pDirStr, "2"))
    {
        if (!(g_driver_axis_status[axis] & FLAG_SERVO_ON))
        {
            EziServo_SetEnable(&g_ezi_servo[axis], true);
            HAL_Delay(10);
        }
        res = EziServo_MoveVelocity(&g_ezi_servo[axis], speed, 0);
        g_jog_state[axis] = JOG_STATE_REVERSE;
        CMD_Printf(";AXIS=%s;DIR=CCW;SPEED=%lu", Get_AxisName(axis), (unsigned long)speed);
    }
    else if (CMD_Compare(pDirStr, "STOP") || CMD_Compare(pDirStr, "0"))
    {
        res = EziServo_MoveStop(&g_ezi_servo[axis]);
        g_jog_state[axis] = JOG_STATE_IDLE;
        CMD_Printf(";AXIS=%s;DIR=STOP", Get_AxisName(axis));
    }
    else
    {
        CMD_Printf(";ERR_PARAM=Unknown Jog direction '%s'", pDirStr);
        return 0;
    }

    return (res == EZI_OK) ? 1 : 0;
}

/**
  * @brief  Set Default Jog/Motion Speed
  *         Syntax: MOTOR.SET_SPEED <axis:X|Y|Z|ALL> <speed_pps>
  */
static u8 Cmd_Motor_SetSpeed(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    int axis;
    u32 speed;

    if (pData->argc < 2)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.SET_SPEED <axis:X|Y|Z|ALL> <speed_pps>");
        return 0;
    }

    axis = Parse_Axis(pData->pArgv[0], -1);
    speed = CMD_StrToUL(pData->pArgv[1]);

    if (axis == AXIS_ALL)
    {
        for (int i = 0; i < NUM_AXES; i++) g_jog_speed_pps[i] = speed;
        CMD_Printf(";AXIS=ALL;SPEED=%lu", (unsigned long)speed);
        return 1;
    }
    else if (axis >= 0 && axis < NUM_AXES)
    {
        g_jog_speed_pps[axis] = speed;
        CMD_Printf(";AXIS=%s;SPEED=%lu", Get_AxisName(axis), (unsigned long)speed);
        return 1;
    }

    CMD_Printf(";ERR_PARAM=Invalid axis '%s'. Use X, Y, Z, or ALL", pData->pArgv[0]);
    return 0;
}

/**
  * @brief  Get / Set Operation Mode (MANUAL vs AUTO)
  *         Syntax: MOTOR.SET_MODE <0=MANUAL|1=AUTO|TOGGLE>
  *                 MOTOR.MODE [0=MANUAL|1=AUTO|TOGGLE]
  */
static u8 Cmd_Motor_SetMode(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData->argc < 1)
    {
        const char *pPending = "NONE";
        if (g_mode_switch_pending == MODE_SWITCH_TO_MANUAL) pPending = "WAITING_AUTO_CYCLE_FINISH";
        else if (g_mode_switch_pending == MODE_SWITCH_TO_AUTO) pPending = "WAITING_MANUAL_FINISH";

        CMD_Printf(";MODE=%s;PENDING=%s", 
                   (g_operation_mode == 0) ? "MANUAL" : "AUTO", pPending);
        return 1;
    }

    uint32_t target_mode = 0;
    if (CMD_Compare(pData->pArgv[0], "AUTO") || CMD_Compare(pData->pArgv[0], "1"))
    {
        target_mode = OP_MODE_AUTO;
    }
    else if (CMD_Compare(pData->pArgv[0], "TOGGLE") || CMD_Compare(pData->pArgv[0], "SW"))
    {
        target_mode = (g_operation_mode == 0) ? OP_MODE_AUTO : OP_MODE_MANUAL;
    }
    else
    {
        target_mode = OP_MODE_MANUAL;
    }

    Request_OperationModeSwitch(target_mode);

    if (g_mode_switch_pending == MODE_SWITCH_TO_MANUAL)
    {
        CMD_Printf(";MODE=%s;STATUS=PENDING_AUTO_CYCLE_FINISH", 
                   (g_operation_mode == 0) ? "MANUAL" : "AUTO");
    }
    else if (g_mode_switch_pending == MODE_SWITCH_TO_AUTO)
    {
        CMD_Printf(";MODE=%s;STATUS=PENDING_MANUAL_FINISH", 
                   (g_operation_mode == 0) ? "MANUAL" : "AUTO");
    }
    else
    {
        CMD_Printf(";MODE=%s;STATUS=SWITCHED", 
                   (g_operation_mode == 0) ? "MANUAL" : "AUTO");
    }
    return 1;
}

static u8 Cmd_Motor_GetMode(void *pVoid)
{
    (void)pVoid;
    const char *pPending = "NONE";
    if (g_mode_switch_pending == MODE_SWITCH_TO_MANUAL) pPending = "WAITING_AUTO_CYCLE_FINISH";
    else if (g_mode_switch_pending == MODE_SWITCH_TO_AUTO) pPending = "WAITING_MANUAL_FINISH";

    CMD_Printf(";MODE=%s;PENDING=%s", 
               (g_operation_mode == 0) ? "MANUAL" : "AUTO", pPending);
    return 1;
}

/**
  * @brief  Control 3-Axis Auto Cycle
  *         Syntax: MOTOR.AUTO_CYCLE <0=STOP|1=SINGLE|2=CONTINUOUS>
  */
static u8 Cmd_Motor_AutoCycle(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData->argc < 1)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.AUTO_CYCLE <0=STOP|1=SINGLE|2=CONTINUOUS>");
        return 0;
    }

    g_auto_cycle_trigger = CMD_StrToUL(pData->pArgv[0]);
    if (g_auto_cycle_trigger > 0)
    {
        Request_OperationModeSwitch(OP_MODE_AUTO);
        g_auto_step = 0;
    }
    else
    {
        g_auto_step = 0;
        g_mode_switch_pending = MODE_SWITCH_NONE;
    }
    CMD_Printf(";TRIGGER=%lu;CYCLE_COUNT=%lu;MODE=%s", 
               (unsigned long)g_auto_cycle_trigger, 
               (unsigned long)g_auto_cycle_count,
               (g_operation_mode == 0) ? "MANUAL" : "AUTO");
    return 1;
}

/**
  * @brief  Configure 3-Axis Coordinates & Parameters for Dispensing Machine
  *         Syntax: MOTOR.AUTO_CONFIG <x_work> <y_disp> <z_out> [disp_time_ms]
  */
static u8 Cmd_Motor_AutoConfig(void *pVoid)
{
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;

    if (pData->argc < 3)
    {
        CMD_Printf(";ERR_USAGE=Usage: MOTOR.AUTO_CONFIG <x_work_pos> <y_disp_pos> <z_out_pos> [disp_time_ms]");
        return 0;
    }

    g_pos_x_work = CMD_StrToL(pData->pArgv[0]);
    g_pos_y_disp = CMD_StrToL(pData->pArgv[1]);
    g_pos_z_out  = CMD_StrToL(pData->pArgv[2]);

    if (pData->argc >= 4)
    {
        g_dispense_time_ms = CMD_StrToUL(pData->pArgv[3]);
    }

    CMD_Printf(";X_WORK=%ld;Y_DISP=%ld;Z_OUT=%ld;DISP_TIME_MS=%lu", 
               (long)g_pos_x_work, (long)g_pos_y_disp, (long)g_pos_z_out, (unsigned long)g_dispense_time_ms);
    return 1;
}
