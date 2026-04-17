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

#include <stdbool.h>

#include "Uart2.h"
#include "Task.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define APP_START_ADDR (0x0002C000)
#define WDOG_EN        0
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

    Uart2_vInit();
    Task_vInit();
    // FlashDrive_vInit();

#if 0
    uint32_t jump_addr = APP_START_ADDR;

    // 只读取 Reset_Handler 地址（不读 MSP）
    uint32_t reset_handler_addr = *(volatile uint32_t *)(jump_addr + 4);

    // // 安全检查（可选）
    // if ((reset_handler_addr & 0x01U) == 0 || reset_handler_addr < jump_addr)
    // {
    //     // 非法 → 进入升级模式
    //     Enter_Upgrade_Mode();
    //     return 0; // 或 while(1)
    // }

    // 跳转前强制 VTOR = 0
    SCB->VTOR = 0x00000000UL;

    // 清除 pending 中断
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    // 关中断（可选）
    __disable_irq();

    // 直接跳转到 App 的 Reset_Handler
    ((void (*)(void))reset_handler_addr)();
#endif

    while (true);
}
/*****************************************************************************
 * End file main.c
 *****************************************************************************/
