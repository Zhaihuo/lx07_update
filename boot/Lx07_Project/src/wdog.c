/*****************************************************************************
 * @file wdog.c
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
#include "wdog.h"

/*****************************************************************************
 * Local macros
 *****************************************************************************/

/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/

/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/

/*****************************************************************************
 * function definitions
 *****************************************************************************/
static void WdogConfig(void)
{
    WDOG_Config_t tWDOGInitConfig;

    tWDOGInitConfig.clkSource    = WDOG_LPO_CLOCK;
    tWDOGInitConfig.wait         = DISABLE;
    tWDOGInitConfig.stop         = DISABLE;
    tWDOGInitConfig.debug        = DISABLE;
    tWDOGInitConfig.winEnable    = DISABLE;
    tWDOGInitConfig.timeoutValue = 0x17700; // 0xFFFF;	//32khz LPO, 1/32K*0xFFFF
    tWDOGInitConfig.windowValue  = 0x0;
    tWDOGInitConfig.testMode     = WDOG_TST_NORMAL;

    WDOG_Init(&tWDOGInitConfig);
}

void Wdog_Init(void)
{
    SYSCTRL_ResetModule(SYSCTRL_WDOG);
    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WdogConfig();

    /* Set wdog callback function*/
    // WDOG_InstallCallBackFunc(WDOG_INT_BEFORERESET, WDOGTEST_IntCallBack);
    /* Enable WDOG interrupt */
    WDOG_IntMask(WDOG_INT_BEFORERESET, UNMASK);
    /* Enable Wdog interrupt */
    NVIC_EnableIRQ(WDOG_IRQn);

    /* Enable wdog */
    WDOG_Enable();
}
/*****************************************************************************
 * End file wdog.c
 *****************************************************************************/
