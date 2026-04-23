/*****************************************************************************
 * @file Task.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2026-04-17
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "Task.h"

#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_uart.h"
#include "Z20K11xM_srmc.h"
#include "Z20K11xM_pmu.h"
#include "Z20K11xM_stim.h"

#include "Uart2.h"
#include "Update.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/

/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
/* STIM configuration */
const STIM_Config_t stimConfig = // 10ms
    {
        .workMode       = STIM_FREE_COUNT,
        .compareValue   = 64000 * 10, /*counter clock is 64M, compare value =64000,  period = 1ms*/
        .countResetMode = STIM_INCREASE_FROM_0,
        .clockSource    = STIM_FUNCTION_CLOCK,
};
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/

/*****************************************************************************
 * function definitions
 *****************************************************************************/
void Task_vInt10ms(void)
{
    // UART_PRINTF("start\r\n");
    Update_vHandle();
}

void Task_vInit(void)
{
    /* Configure STIM function clock*/
    CLK_ModuleSrc(CLK_STIM, CLK_SRC_FIRC64M);

    /* Enable STIM module */
    SYSCTRL_EnableModule(SYSCTRL_STIM);

    /* Init STIM_0*/
    STIM_Init(STIM_0, &stimConfig);

    /* Install interrupt callback function */
    STIM_InstallCallBackFunc(STIM_0, STIM_INT, Task_vInt10ms);
    /* Enable STIM_0 interrupt*/
    STIM_IntCmd(STIM_0, ENABLE);

    /*Enable STIM*/
    STIM_Enable(STIM_0);
    /* Enable STIM NVIC IRQ*/
    NVIC_EnableIRQ(STIM_IRQn);
}
/*****************************************************************************
 * End file Task.c
 *****************************************************************************/
