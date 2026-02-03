/**************************************************************************************************/
/**
 * @file     wdog.c
 * @brief
 * @version  V1.0.0
 * @date     December-2023
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/

#include "wdog.h"

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