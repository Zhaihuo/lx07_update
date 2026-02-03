/**************************************************************************************************/
/**
 * @file      : main.c
 * @brief     :
 * @version   : V1.8.0
 * @date      : December-2023
 * @author    :
 *
 * @note      : This example contains sample code for customer evaluation purpose only. It is not
 * part of the production code deliverables. The example code is only tested under defined
 * environment with related context of the whole example project. Therefore, it is not guaranteed
 * that the example always works under user environment due to the diversity of hardware and
 * software environment. Do not use pieces copy of the example code without prior and separate
 * verification and validation in user environment.
 *
 * @copyright : Copyright (c) 2020-2023 Zhixin Semiconductor Ltd. All rights reserved.
 **************************************************************************************************/

#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_uart.h"

#include "serial.h"
#include "i2c.h"
#include "uart.h"
#include "wdog.h"
#include "SysTick.h"
#include "Scheduler.h"
#include "can.h"
#include "Uart2.h"
#include "Config.h"
#include "Debounce.h"
#include "touch.h"

#define WDOG_EN 0

__STATIC_INLINE void SysTick_Init(void)
{
    SysTick->LOAD = 0x00FFFFFEu; /* set reload register */
    SysTick->VAL  = 0UL;         /* Load the SysTick Counter Value */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk; /* Enable SysTick IRQ and SysTick Timer */
}

/* delay function*/
void delay(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while (cycles--);
}

void PortCInt(PORT_ID_t portId, PORT_GPIONO_t gpioNo);

void GPIOIntInit(void)
{
    PORT_InstallCallBackFunc(PortCInt);

    // /* v1.0.1 */
    /* Clear interrupt flag*/
    // PORT_ClearPinInt(PORT_C, GPIO_1);
    // /* set pin as GPIO*/
    // PORT_PinmuxConfig(PORT_C, GPIO_1, PTC1_GPIO);
    // /* an initial voltage */
    // PORT_PullConfig(PORT_C, GPIO_1, PORT_PULL_DOWN);
    // /* input direction for PTC1 */
    // GPIO_SetPinDir(PORT_C, GPIO_1, GPIO_INPUT);
    // /* port interrupt config*/
    // PORT_PinIntConfig(PORT_C, GPIO_1, PORT_ISF_INT_RISING_EDGE);

    /* v1.0.3 */
    /* Clear interrupt flag*/ // sw1
    PORT_ClearPinInt(PORT_D, GPIO_2);
    /* set pin as GPIO*/
    PORT_PinmuxConfig(PORT_D, GPIO_2, PTD2_GPIO);
    /* an initial voltage */
    PORT_PullConfig(PORT_D, GPIO_2, PORT_PULL_DOWN);
    /* input direction for PTC1 */
    GPIO_SetPinDir(PORT_D, GPIO_2, GPIO_INPUT);
    /* port interrupt config*/
    PORT_PinIntConfig(PORT_D, GPIO_2, PORT_ISF_INT_RISING_EDGE);

    /* Clear interrupt flag*/ // sw2
    PORT_ClearPinInt(PORT_C, GPIO_2);
    /* set pin as GPIO*/
    PORT_PinmuxConfig(PORT_C, GPIO_2, PTC2_GPIO);
    /* an initial voltage */
    PORT_PullConfig(PORT_C, GPIO_2, PORT_PULL_DOWN);
    /* input direction for PTC2 */
    GPIO_SetPinDir(PORT_C, GPIO_2, GPIO_INPUT);
    /* port interrupt config*/
    PORT_PinIntConfig(PORT_C, GPIO_2, PORT_ISF_INT_RISING_EDGE);

    // /* Clear interrupt flag*///sw3
    PORT_ClearPinInt(PORT_C, GPIO_3);
    /* set pin as GPIO*/
    PORT_PinmuxConfig(PORT_C, GPIO_3, PTC3_GPIO);
    /* an initial voltage */
    PORT_PullConfig(PORT_C, GPIO_3, PORT_PULL_DOWN);
    /* input direction for PTC3 */
    GPIO_SetPinDir(PORT_C, GPIO_3, GPIO_INPUT);
    /* port interrupt config*/
    PORT_PinIntConfig(PORT_C, GPIO_3, PORT_ISF_INT_RISING_EDGE);

    /* v1.0.3 TP */
    /* Clear interrupt flag*/
    PORT_ClearPinInt(PORT_C, GPIO_8);
    /* set pin as GPIO*/
    PORT_PinmuxConfig(PORT_C, GPIO_8, PTC8_GPIO);
    /* an initial voltage */
    PORT_PullConfig(PORT_C, GPIO_8, PORT_PULL_UP);
    /* input direction for PTC8 */
    GPIO_SetPinDir(PORT_C, GPIO_8, GPIO_INPUT);
/* port interrupt config*/
#if 0
    PORT_PinIntConfig(PORT_C, GPIO_8, PORT_ISF_INT_RISING_EDGE);
#else // Lx07
    PORT_PinIntConfig(PORT_C, GPIO_8, PORT_ISF_INT_FALLING_EDGE);
#endif

    NVIC_SetPriority(PORTABC_IRQn, 1u);
    /* enable NVIC IRQ*/
    NVIC_EnableIRQ(PORTABC_IRQn);
    NVIC_EnableIRQ(PORTDE_IRQn);
}

void PortCInt(PORT_ID_t portId, PORT_GPIONO_t gpioNo)
{
    if ((PORT_C == portId) && (GPIO_8 == gpioNo))
    {
#ifdef TOUCH_NEED_TPCOUNT
        Debounce_vInTpIRQ(&Debounce_StTpTrigger);
#else
        // Debounce_vInIRQ(&Debounce_StTpTrigger);
#endif
        Touch_boMisTpIntFlg = true;

        // fts_gpio_interrupt_handler();
    }
    else if ((PORT_D == portId) && (GPIO_2 == gpioNo)) // Sw1
    {
        Debounce_vInIRQ(&Debounce_StSw1);
    }
    else if ((PORT_C == portId) && (GPIO_2 == gpioNo)) // Sw2
    {
        Debounce_vInIRQ(&Debounce_StSw2);
    }
    else if ((PORT_C == portId) && (GPIO_3 == gpioNo)) // Sw3
    {
        Debounce_vInIRQ(&Debounce_StSw3);
    }
}

void System_Init(void)
{
    /* Disable wdog */
    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();

    /* Enable OSC40M clock*/
    CLK_OSC40MEnable2(CLK_OSC_FREQ_MODE_HIGH, ENABLE, CLK_OSC_XTAL);
    /* Select OSC40M as system clock*/
    CLK_SysClkSrc(CLK_SYS_FIRC64M);         // zx review , CLK_SYS_OSC40M -> CLK_SYS_FIRC64M
    CLK_SetClkDivider(CLK_CORE, CLK_DIV_1); // zx review
    CLK_SetClkDivider(CLK_BUS, CLK_DIV_2);  // zx review
    CLK_SetClkDivider(CLK_SLOW, CLK_DIV_8); // zx review
}

void Common_Init(void)
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

    /* enable GPIO module*/
    SYSCTRL_EnableModule(SYSCTRL_GPIO);
}

void Peripheral_Init(void)
{
    I2c_Init();

    Uart0_Init();

    Uart2_vInit();

    SysTickInit();

    CANConfig_Init();
}

int main()
{
    /* system init */
    System_Init();

    /* common module init */
    Common_Init();

#ifdef WDOG_EN
    // delay(30000);		//test delay , avoid frequent reset of the mcu
    Wdog_Init();
#endif

#if 1 // Lx07
    GPIOIntInit();
#endif

    // WDOG_Disable();

    // /* pull up deserialuzer power v1.0.1 */
    // PORT_PinmuxConfig(PORT_A, GPIO_11, PTA11_GPIO);
    // GPIO_SetPinDir(PORT_A, GPIO_11, GPIO_OUTPUT);
    // GPIO_SetPinOutput(PORT_A, GPIO_11);

    /* pull up deserialuzer power v1.0.3 */
    PORT_PinmuxConfig(PORT_A, GPIO_13, PTA13_GPIO);
    GPIO_SetPinDir(PORT_A, GPIO_13, GPIO_OUTPUT);
    GPIO_SetPinOutput(PORT_A, GPIO_13);

    PORT_PinmuxConfig(PORT_A, GPIO_12, PTA12_GPIO);
    GPIO_SetPinDir(PORT_A, GPIO_12, GPIO_OUTPUT);
    GPIO_SetPinOutput(PORT_A, GPIO_12);

    /* set pin as GPIO*/
    PORT_PinmuxConfig(PORT_C, GPIO_5, PTC5_GPIO);
    /* an initial voltage */
    PORT_PullConfig(PORT_C, GPIO_5, PORT_PULL_DOWN);

    /* input direction for PTC5 */
    GPIO_SetPinDir(PORT_C, GPIO_5, GPIO_INPUT);

    Peripheral_Init();

    Sch_Main();
}
