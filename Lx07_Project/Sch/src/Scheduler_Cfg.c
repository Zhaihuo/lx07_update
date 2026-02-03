#include "Scheduler.h"
#include "Scheduler_Cfg.h"
#include "uart.h"
#include "wdog.h"
#include "Config.h"
#include "can.h"
#include "i2c.h"
#include "Lx07.h"

void Task1(void)
{
}

void Task2(void)
{
}

void Task3(void)
{
}

void Task4(void)
{
}

void Task5(void)
{
}

void Task6(void)
{
}

void Task7(void)
{
}

void Task8(void)
{
}

void Task9(void)
{
}

void Task10(void)
{
}

void Task11(void)
{
}

void Task12(void)
{
}

void Task13(void)
{
}

const Sch_Task_Info_T Sch_RunTaskTable[] =
    {
        /* always online tasks: ------------------------------------------*/
        {&Task12,        SCH_MS(0u),  SCH_TIMEDURATION(0u) },
        {SCH_EndOfList,  SCH_MS(1u),  SCH_TIMEDURATION(0u) },
        /* 1ms periodic tasks: -------------------------------------------*/
        {&Task11,        SCH_MS(0u),  SCH_TIMEDURATION(1u) },
        {SCH_EndOfList,  SCH_MS(1u),  SCH_TIMEDURATION(1u) },
        /*5ms periodic tasks: -------------------------------------------*/
        {&Lx07_vTask5ms, SCH_MS(0u),  SCH_TIMEDURATION(5u) },
        {SCH_EndOfList,  SCH_MS(5u),  SCH_TIMEDURATION(5u) },
        /* 10ms periodic tasks: -------------------------------------------*/
        {&Task1,         SCH_MS(0u),  SCH_TIMEDURATION(1u) },
        {&Task2,         SCH_MS(1u),  SCH_TIMEDURATION(2u) },
        {&Task3,         SCH_MS(2u),  SCH_TIMEDURATION(3u) },
        {&Task4,         SCH_MS(3u),  SCH_TIMEDURATION(4u) },
        {&Task5,         SCH_MS(4u),  SCH_TIMEDURATION(5u) },
        {&Task6,         SCH_MS(5u),  SCH_TIMEDURATION(5u) },
        {&Task7,         SCH_MS(6u),  SCH_TIMEDURATION(7u) },
        {&Task8,         SCH_MS(7u),  SCH_TIMEDURATION(8u) },
        {&Task9,         SCH_MS(8u),  SCH_TIMEDURATION(9u) },
        {&Task10,        SCH_MS(9u),  SCH_TIMEDURATION(10u)},
        {SCH_EndOfList,  SCH_MS(10u), SCH_TIMEDURATION(10u)},
        /* 40ms periodic tasks: -------------------------------------------*/
        {&Task13,        SCH_MS(0u),  SCH_TIMEDURATION(40u)},
        {SCH_EndOfList,  SCH_MS(40u), SCH_TIMEDURATION(40u)},
        {SCH_EndOfList,  SCH_MS(0u),  SCH_TIMEDURATION(0u) }
};
