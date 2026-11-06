/**
  ******************************************************************************
  * @file    CmdSet_Function.c
  * @brief   Command Parsing, Formatting and Execution Engine
  *          Referenced from TOP Engineering PG_StudyProject_Test
  ******************************************************************************
  */

#include "CmdSet_Function.h"
#include "CmdSet_Process.h"

static char *s_cmdArgv[CMDSET_MAX_COUNT];
static u32  s_cmdArgc = 0;

static u8 CMD_Parsing(char *pRcvData, char **pArgv, u32 *pArgc)
{
    u8   result = 0;
    char *pToken, *pData, *pLastData;
    u32  argc = 0, length, totalLength;

    if (pRcvData == NULL || pArgv == NULL || pArgc == NULL) return 0;

    totalLength = (u32)strlen(pRcvData);
    if (totalLength == 0) return 0;

    pLastData = &pRcvData[totalLength];

    /* First token split by DELIMITER_1 (space, dot, comma, tab, \r, \n) */
    pToken = strtok(pRcvData, " .,\t\r\n");

    while (pToken != NULL)
    {
        pArgv[argc++] = pToken;
        if (argc >= CMDSET_MAX_COUNT) break;

        length = (u32)strlen(pToken) + 1;

        /* Skip consecutive spaces / tabs */
        while (&pToken[length] < pLastData)
        {
            if (pToken[length] == ' ' || pToken[length] == '\t' || pToken[length] == ',')
            {
                pToken[length] = '\0';
                length++;
            }
            else
            {
                break;
            }
        }

        pData = &pToken[length];
        if (pData >= pLastData) break;

        /* Check for quoted string */
        if (*pData == '"')
        {
            pToken = strtok(pData, "\"");
        }
        else
        {
            pToken = strtok(pData, " .,\t\r\n");
        }
    }

    if (argc != 0)
    {
        *pArgc = argc;
        result = 1;
    }

    return result;
}

u8 CMD_Analysis(void)
{
    u8   result = 0, rtn;
    char *pComment;

    memset(s_cmdArgv, 0, sizeof(s_cmdArgv));
    s_cmdArgc = 0;

    /* Strip comments starting with '#' or ';' */
    pComment = strstr((char *)cmdSetHandle.pRxData, "#");
    if (pComment != NULL)
    {
        *pComment = '\0';
    }

    rtn = CMD_Parsing((char *)cmdSetHandle.pRxData, s_cmdArgv, &s_cmdArgc);
    if (rtn != 0)
    {
        result = 1;
    }

    return result;
}

u32 CMD_GetCommunicationType(void)
{
    return cmdSetHandle.rxSrcType;
}

u8 CMD_RunningState(u8 state)
{
    if (cmdSetHandle.RunningState != NULL)
    {
        return cmdSetHandle.RunningState(state);
    }
    return 1;
}

u8 CMD_ErrorState(u8 state)
{
    if (cmdSetHandle.ErrorState != NULL)
    {
        return cmdSetHandle.ErrorState(state);
    }
    return 1;
}

u8 CMD_DataSplit(char *pString, const char *pDelimiter, char **pArgv, u32 argvMaxCount, u32 *pArgc)
{
    u8   result = 0;
    char *pToken;
    u32  argc = 0;

    if (pString == NULL || pDelimiter == NULL || pArgv == NULL) return 0;

    pToken = strtok(pString, pDelimiter);
    while (pToken != NULL)
    {
        pArgv[argc++] = pToken;
        if (argc >= argvMaxCount) break;
        pToken = strtok(NULL, pDelimiter);
    }

    if (argc != 0) result = 1;
    if (pArgc != NULL) *pArgc = argc;

    return result;
}

u8 CMD_Compare(const char *pS1, const char *pS2)
{
    if (pS1 == NULL || pS2 == NULL) return 0;

    while (*pS1 && *pS2)
    {
        char c1 = *pS1++;
        char c2 = *pS2++;

        if (c1 >= 'a' && c1 <= 'z') c1 -= 0x20;
        if (c2 >= 'a' && c2 <= 'z') c2 -= 0x20;

        if (c1 != c2) return 0;
    }

    return (*pS1 == '\0' && *pS2 == '\0') ? 1 : 0;
}

u8 CMD_StrLwr(char *pData)
{
    if (pData == NULL) return 0;
    while (*pData)
    {
        if (*pData >= 'A' && *pData <= 'Z')
        {
            *pData += 0x20;
        }
        pData++;
    }
    return 1;
}

u8 CMD_StrUpr(char *pData)
{
    if (pData == NULL) return 0;
    while (*pData)
    {
        if (*pData >= 'a' && *pData <= 'z')
        {
            *pData -= 0x20;
        }
        pData++;
    }
    return 1;
}

u32 CMD_StrToUL(const char *pData)
{
    if (pData == NULL) return 0;
    return (u32)strtoul(pData, NULL, 0);
}

s32 CMD_StrToL(const char *pData)
{
    if (pData == NULL) return 0;
    return (s32)strtol(pData, NULL, 0);
}

double CMD_AToF(const char *pData)
{
    if (pData == NULL) return 0.0;
    return atof(pData);
}

u8 CMD_TxData(u8 *pData, u32 length)
{
    if ((cmdSetHandle.txDataCnt + length) > cmdSetHandle.txDataSize)
    {
        return 0;
    }

    memcpy(&cmdSetHandle.pTxData[cmdSetHandle.txDataCnt], pData, length);
    cmdSetHandle.txDataCnt += length;
    return 1;
}

u8 CMD_Printf(const char *pFormat, ...)
{
    va_list ap;
    va_start(ap, pFormat);

    if ((cmdSetHandle.txDataCnt + 64) > cmdSetHandle.txDataSize)
    {
        va_end(ap);
        return 0;
    }

    char *pwData = (char *)&cmdSetHandle.pTxData[cmdSetHandle.txDataCnt];
    int length = vsprintf(pwData, pFormat, ap);
    va_end(ap);

    if (length > 0)
    {
        cmdSetHandle.txDataCnt += (u32)length;
        cmdSetHandle.pTxData[cmdSetHandle.txDataCnt] = '\0';
    }

    return 1;
}

u8 CMD_TransmitData(void)
{
    u32 position = cmdSetHandle.txSrcType;

    if (position >= CMDSET_COMM_SRC_COUNT) return 0;
    if (cmdSetHandle.TransmitData[position] == NULL) return 0;

    return cmdSetHandle.TransmitData[position](cmdSetHandle.pTxData, cmdSetHandle.txDataCnt);
}

u8 CMD_Execute(void)
{
    u8  result = 0, rtn = 0;
    u32 cmdCnt = 0;
    cmdFunctionData_t functionData;

    if (s_cmdArgc == 0) return 0;
    if (cmdSetHandle.pCmdList == NULL) return 0;

    /* Search in Root Command List */
    while (1)
    {
        if (cmdSetHandle.pCmdList[cmdCnt].pCmd == NULL)
        {
            CMD_Printf(">>NO_COMMAND;ERROR\r\n");
            return 0;
        }

        if (CMD_Compare(s_cmdArgv[0], cmdSetHandle.pCmdList[cmdCnt].pCmd))
        {
            break;
        }
        cmdCnt++;
    }

    memset(&functionData, 0, sizeof(functionData));
    functionData.srcType    = cmdSetHandle.txSrcType;
    functionData.pCmdList   = (commandList_t *)cmdSetHandle.pCmdList;
    functionData.pArgv      = &s_cmdArgv[1];
    functionData.argc       = s_cmdArgc - 1;
    functionData.option     = cmdSetHandle.pCmdList[cmdCnt].option;
    functionData.retryCnt   = 0;
    functionData.deactivate = (cmdSetHandle.pCmdList[cmdCnt].option != 0) ? 1 : 0;
    functionData.channel    = 0;
    functionData.position   = 0;

    /* Print Command Header */
    if (cmdSetHandle.pCmdList[cmdCnt].pMessage != NULL)
    {
        CMD_Printf("%s", cmdSetHandle.pCmdList[cmdCnt].pMessage);
    }
    else
    {
        CMD_Printf(">>%s", cmdSetHandle.pCmdList[cmdCnt].pCmd);
    }

    /* Execute Command Handler */
    if (cmdSetHandle.pCmdList[cmdCnt].pFunction != NULL)
    {
        rtn = cmdSetHandle.pCmdList[cmdCnt].pFunction(&functionData);
    }
    else
    {
        CMD_Printf(";NO_FUNCTION");
        rtn = 0;
    }

    /* Append Final Status and Line Ending */
    if (rtn != 0)
    {
        CMD_Printf(";OK\r\n");
        result = 1;
    }
    else
    {
        CMD_Printf(";ERROR\r\n");
    }

    return result;
}

u8 CMD_SubCommandExecute(void *pvFuncData, const void *pvCmdList)
{
    u8  result = 0;
    u32 cmdCnt = 0;
    cmdFunctionData_t *pParentData = (cmdFunctionData_t *)pvFuncData;
    const commandList_t *pSubList = (const commandList_t *)pvCmdList;
    cmdFunctionData_t subFunctionData;

    if (pvFuncData == NULL || pvCmdList == NULL) return 0;
    if (pParentData->argc == 0)
    {
        CMD_Printf(";NO_ARGUMENT");
        return 0;
    }

    /* Match sub-command token */
    while (1)
    {
        if (pSubList[cmdCnt].pCmd == NULL)
        {
            if (pParentData->retryCnt == 0)
            {
                CMD_Printf(";UNKNOWN_SUBCMD=%s", pParentData->pArgv[0]);
                pParentData->retryCnt++;
            }
            return 0;
        }

        if (CMD_Compare(pParentData->pArgv[0], pSubList[cmdCnt].pCmd))
        {
            break;
        }
        cmdCnt++;
    }

    /* Build sub-command argument context */
    memset(&subFunctionData, 0, sizeof(subFunctionData));
    subFunctionData.srcType    = pParentData->srcType;
    subFunctionData.pCmdList   = (commandList_t *)pvCmdList;
    subFunctionData.pArgv      = &pParentData->pArgv[1];
    subFunctionData.argc       = pParentData->argc - 1;
    subFunctionData.option     = pSubList[cmdCnt].option;
    subFunctionData.channel    = pParentData->channel;
    subFunctionData.position   = pParentData->position;

    /* Print sub-message if defined */
    if (pSubList[cmdCnt].pMessage != NULL && pParentData->retryCnt == 0)
    {
        CMD_Printf("%s", pSubList[cmdCnt].pMessage);
    }

    pParentData->retryCnt++;

    if (pSubList[cmdCnt].pFunction != NULL)
    {
        result = pSubList[cmdCnt].pFunction(&subFunctionData);
    }
    else
    {
        CMD_Printf(";NO_FUNCTION");
        result = 0;
    }

    return result;
}

u8 Cmd_Help(void *pVoid)
{
    u32 cnt = 0;
    cmdFunctionData_t *pData = (cmdFunctionData_t *)pVoid;
    const commandList_t *pCmdList = (pData != NULL) ? pData->pCmdList : cmdSetHandle.pCmdList;

    if (pCmdList == NULL) return 0;

    if (pData == NULL || pData->argc == 0)
    {
        /* List all commands */
        CMD_Printf(";CMD_LIST=");
        while (pCmdList[cnt].pCmd != NULL)
        {
            if (pCmdList[cnt].pFunction != NULL)
            {
                if (cnt == 0)
                {
                    CMD_Printf("%s", pCmdList[cnt].pCmd);
                }
                else
                {
                    CMD_Printf(", %s", pCmdList[cnt].pCmd);
                }
            }
            cnt++;
        }
        return 1;
    }
    else if (pData->argc >= 1)
    {
        /* Detail specific command help */
        while (pCmdList[cnt].pCmd != NULL)
        {
            if (CMD_Compare(pData->pArgv[0], pCmdList[cnt].pCmd))
            {
                if (pCmdList[cnt].pNote != NULL)
                {
                    CMD_Printf(";%s=%s", pCmdList[cnt].pCmd, pCmdList[cnt].pNote);
                }
                else
                {
                    CMD_Printf(";%s=NO_NOTE", pCmdList[cnt].pCmd);
                }
                return 1;
            }
            cnt++;
        }
        CMD_Printf(";NO_COMMAND");
        return 0;
    }

    return 0;
}
