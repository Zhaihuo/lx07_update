#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "stdint.h"
#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"

extern void SysTick_GetCurCount(uint32_t *u32Value);
extern void SysTick_SetCount(uint32_t u32Value);
extern void SysTick_GetOverF(uint8_t *u8Value);
extern void SysTickInit(void);

#endif
