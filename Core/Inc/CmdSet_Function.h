/**
  ******************************************************************************
  * @file    CmdSet_Function.h
  * @brief   Command Parsing, Formatting and Sub-command Execution Prototypes
  *          Referenced from TOP Engineering PG_StudyProject_Test
  ******************************************************************************
  */

#ifndef _CMDSET_FUNCTION_H_
#define _CMDSET_FUNCTION_H_

#include "CommandSet.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMMAND_DELIMITER_1     " ,\t\r\n"
#define COMMAND_DELIMITER_2     " ,\t\r\n"
#define COMMAND_DELIMITER_3     "\""

u32     CMD_GetCommunicationType(void);
u8      CMD_RunningState(u8 state);
u8      CMD_ErrorState(u8 state);
u8      CMD_DataSplit(char *pString, const char *pDelimiter, char **pArgv, u32 argvMaxCount, u32 *pArgc);
u8      CMD_Compare(const char *pS1, const char *pS2);
u8      CMD_StrLwr(char *pData);
u8      CMD_StrUpr(char *pData);
u32     CMD_StrToUL(const char *pData);
s32     CMD_StrToL(const char *pData);
double  CMD_AToF(const char *pData);
u8      CMD_TxData(u8 *pData, u32 length);
u8      CMD_Printf(const char *pFormat, ...);
u8      CMD_TransmitData(void);
u8      CMD_Execute(void);
u8      CMD_SubCommandExecute(void *pvFuncData, const void *pvCmdList);
u8      Cmd_Help(void *pVoid);

#ifdef __cplusplus
}
#endif

#endif /* _CMDSET_FUNCTION_H_ */
