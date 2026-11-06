#ifndef	_OSHOOK_H
#define	_OSHOOK_H

#include	"f769Disco.h"

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vAssertCalled( uint32_t ulLine, const char *pcFile );
void vApplicationTickHook( void );
void vApplicationMallocFailedHook( void );
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize );

#endif	// _OSHOOK_H