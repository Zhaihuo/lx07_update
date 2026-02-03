#include "SysTick.h"
#include "Scheduler.h"
#include "Scheduler_Cfg.h"

Sch_Const_Task_Info_T Sch_TaskTable;
static TaskTablePara  sch_TaskPara[SCH_PERIODIC_MAX_NUM] = {
    {0u, 0u, 0u, 0u}
};
/* Task's periodic */
static uint8_t  u8PeriodicNum   = 0u;
static uint32_t u32TickVal      = 0u;
static uint32_t u32CurTimeStamp = 0x00FFFFFFu;
static uint32_t u32PreTimeStamp = 0x00FFFFFFu;
static uint32_t u32TimeDuration = 0u;

void Sch_TableInit(Sch_Const_Task_Info_T TaskTable)
{
    uint8_t u8Count = 0u;
    uint8_t u8Idx   = 0u;
    Sch_TaskTable   = TaskTable;
    /* Find out all tasks */
    while ((Sch_TaskTable[u8Count].pTask != SCH_EndOfList) || (Sch_TaskTable[u8Count].u8TimeOffset != 0u))
    {
        /* Get task's start position */
        if ((Sch_TaskTable[u8Count].pTask != SCH_EndOfList) && (Sch_TaskTable[u8Count].u8TimeOffset == SCH_MS(0u)))
        {
            sch_TaskPara[u8Idx].u8TaskStartPosition = u8Count;
        }
        /* Get task's periodic and end position */
        else if ((Sch_TaskTable[u8Count].pTask == SCH_EndOfList) && (Sch_TaskTable[u8Count].u8TimeOffset != SCH_MS(0u)))
        {
            sch_TaskPara[u8Idx].u8TaskEndPosition = u8Count;
            sch_TaskPara[u8Idx].u8CurExecuteIdx   = 0u;
            u8Idx += 1u;
            u8PeriodicNum += 1u;
        }
        /* Task behind the first */
        else
        {
            /* Do nothing */
        }
        u8Count += 1u;
    }
}
/* Systick clock is 64/1Mhz = 64Mhz */
/* 1ms = (1/64Mhz)*n*1000 : n= 64000 */
void Sch_Main(void)
{
    uint8_t u8Count;

    Sch_TableInit(Sch_RunTaskTable);

    while (1)
    {
        SysTick_GetCurCount(&u32CurTimeStamp);
        if (u32CurTimeStamp >= 2u)
        {
            u32CurTimeStamp -= 2u;
        }
        else
        {
            u32CurTimeStamp = 0x00FFFFFFu;
        }
        /* Overflow */
        if (u32CurTimeStamp >= u32PreTimeStamp)
        {
            u32TimeDuration = u32PreTimeStamp + 0x00FFFFFFu - u32CurTimeStamp;
        }
        else if (u32CurTimeStamp < u32PreTimeStamp)
        {
            u32TimeDuration = u32PreTimeStamp - u32CurTimeStamp;
        }
        else
        {
            /* Do nothing */
        }

        u32PreTimeStamp = u32CurTimeStamp;

        for (u8Count = 0u; u8Count < u8PeriodicNum; u8Count++)
        {
            sch_TaskPara[u8Count].u32TimeDuration += u32TimeDuration;
            /* Time is coming */
            if (sch_TaskPara[u8Count].u32TimeDuration >= (Sch_TaskTable[(sch_TaskPara[u8Count].u8TaskStartPosition + sch_TaskPara[u8Count].u8CurExecuteIdx)].u32TimeDuration))
            {
                CALL_TASK(sch_TaskPara[u8Count].u8TaskStartPosition + sch_TaskPara[u8Count].u8CurExecuteIdx);
                if ((sch_TaskPara[u8Count].u8CurExecuteIdx + sch_TaskPara[u8Count].u8TaskStartPosition + 1u) < sch_TaskPara[u8Count].u8TaskEndPosition)
                {
                    sch_TaskPara[u8Count].u8CurExecuteIdx += 1u;
                }
                else
                {
                    sch_TaskPara[u8Count].u8CurExecuteIdx = 0u;
                }
                if (sch_TaskPara[u8Count].u32TimeDuration >= Sch_TaskTable[sch_TaskPara[u8Count].u8TaskEndPosition].u32TimeDuration)
                {
                    sch_TaskPara[u8Count].u32TimeDuration = 0u;
                }
                else
                {
                    /* Do nothing */
                }
            }
            else
            {
                /* Do nothing */
            }
        }
    }
}
