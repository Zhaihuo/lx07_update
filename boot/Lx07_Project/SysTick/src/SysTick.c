#include "SysTick.h"

void SysTickInit(void)
{
    /* MAX counter */
    SysTick->LOAD = 0x00FFFFFFu;
    /* SysTick counter Init value */
    SysTick->VAL = 0;
    /* Enable SysTick Timer */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

void SysTick_GetCurCount(uint32_t *u32Value)
{
    *u32Value = SysTick->VAL;
}

void SysTick_SetCount(uint32_t u32Value)
{
    SysTick->VAL = u32Value;
}

void SysTick_GetOverF(uint8_t *u8Value)
{
    *u8Value = (SysTick->CTRL >> 16u) & 0x01u;
}
 