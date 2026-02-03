#ifndef _SCH_H_
#define _SCH_H_
#include "stdint.h"

typedef void (*Sch_Task_T)(void);

typedef struct Sch_Task_Info_T
{
    Sch_Task_T pTask;
    uint8_t    u8TimeOffset;
    uint32_t   u32TimeDuration;
} Sch_Task_Info_T;

typedef const Sch_Task_Info_T *Sch_Const_Task_Info_T;

#define CALL_TASK(idx) Sch_TaskTable[idx].pTask()

extern void Sch_Main(void);

#endif
