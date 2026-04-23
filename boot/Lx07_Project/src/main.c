/*****************************************************************************
 * @file main.c
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
#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_uart.h"
#include "Z20K11xM_regfile.h"
#include <stdbool.h>
#include "Uart2.h"
#include "Task.h"
#include "Config.h"
#include "FlashDriver.h"
#include "Update.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
// #define APP_START_ADDR (0x0002C000)
#define WDOG_EN 0
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
static void Gpio_vInit(void)
{
    CLK_ModuleSrc(CLK_PORTA, CLK_SRC_OSC40M);
    SYSCTRL_EnableModule(SYSCTRL_PORTA);
    CLK_ModuleSrc(CLK_PORTB, CLK_SRC_OSC40M);
    SYSCTRL_EnableModule(SYSCTRL_PORTB);
    CLK_ModuleSrc(CLK_PORTC, CLK_SRC_OSC40M);
    SYSCTRL_EnableModule(SYSCTRL_PORTC);
    CLK_ModuleSrc(CLK_PORTD, CLK_SRC_OSC40M);
    SYSCTRL_EnableModule(SYSCTRL_PORTD);
    CLK_ModuleSrc(CLK_PORTE, CLK_SRC_OSC40M);
    SYSCTRL_EnableModule(SYSCTRL_PORTE);

    SYSCTRL_EnableModule(SYSCTRL_GPIO);
}

void System_vInit(void)
{
    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();
    CLK_OSC40MEnable2(CLK_OSC_FREQ_MODE_HIGH, ENABLE, CLK_OSC_XTAL);
    CLK_SysClkSrc(CLK_SYS_FIRC64M);
    CLK_SetClkDivider(CLK_CORE, CLK_DIV_1);
    CLK_SetClkDivider(CLK_BUS, CLK_DIV_2);
    CLK_SetClkDivider(CLK_SLOW, CLK_DIV_8);

    Gpio_vInit();
}

int main(void)
{
    __disable_irq();
    System_vInit();
    __enable_irq();

    Update_vInit();

    uint32_t u32ReRegID01Val = 0x00;
    uint32_t u32WrRegID01Val = 0x00;
    REGFILE_ReadByRegID(REGFILE_ID_01, &u32ReRegID01Val);
    if (REGFILE_NORMAL_FLG == u32ReRegID01Val)
    {
        REGFILE_WriteByRegID(REGFILE_ID_01, &u32WrRegID01Val);
        Update_vJumpApp();
    }

    Uart2_vInit();
    Task_vInit();
    FlashDrive_vInit();

    // uint32_t u32BootValye = *(volatile uint32_t *)DFLASH_START;/*从地址读取数据*/

    while (true);
}
/*****************************************************************************
 * End file main.c
 *****************************************************************************/
