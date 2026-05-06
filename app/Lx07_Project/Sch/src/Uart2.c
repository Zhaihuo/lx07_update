/*****************************************************************************
 * @file Uart2.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-10-09
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
#include <stdarg.h> // 用于处理可变参数
#include <string.h> // 用于字符串操作
#include <stdio.h>  // 用于sprintf
/********************************************FF*********************************
 * Local macros
 *****************************************************************************/
#define REGFILE_ID_UPDATE       (0)
#define REGFILE_UPDATE_BOOT_FLG (0x01010101)
#define REGFILE_UPDATE_APP_FLG  (0x02020202)
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
void Uart2_vReadInt(void)
{
    static uint8_t u8Count      = 0;
    static uint8_t au8Buffer[4] = {0};
    uint8_t        u8Data       = UART_ReceiveByte(UART2_ID);

    au8Buffer[u8Count++] = u8Data;

    if (u8Count >= 4)
        u8Count = 0;

    // update
    uint32_t u32AppValue = 0;
    if ((0x01 == au8Buffer[0]) && (0x01 == au8Buffer[1]) && (0x01 == au8Buffer[2]) && (0x01 == au8Buffer[3]))
    {
        u32AppValue = REGFILE_UPDATE_BOOT_FLG;
        REGFILE_WriteByRegID(REGFILE_ID_UPDATE, &u32AppValue);
        NVIC_SystemReset();
    }
    else if ((0x02 == au8Buffer[0]) && (0x02 == au8Buffer[1]) && (0x02 == au8Buffer[2]) && (0x02 == au8Buffer[3]))
    {
        u32AppValue = REGFILE_UPDATE_APP_FLG;
        REGFILE_WriteByRegID(REGFILE_ID_UPDATE, &u32AppValue);
        NVIC_SystemReset();
    }
}

void Uart2_vSend(uint8_t *logBuf, uint32_t logLen)
{
    uint32_t i  = 0;
    char     ch = 0;
    for (i = 0; i < logLen; i++)
    {
        ch = *(logBuf + i);
        /* Wait untill FIFO is empty */
        while (RESET == UART_GetLineStatus(UART2_ID, UART_LINESTA_TEMT));

        /* Send data */
        UART_SendByte(UART2_ID, ch);
    }
}

// void Uart2_vPrintU8Data(uint8_t u8Data) // 0~255
// {
//     uint8_t au8Data[3] = {0};

//     au8Data[0] = u8Data / 100 + 0x30;
//     au8Data[1] = (u8Data % 100) / 10 + 0x30;
//     au8Data[2] = u8Data % 10 + 0x30;
//     Uart2_vSend((const char *)au8Data, sizeof(au8Data));
// }

// void Uart2_vPrintU16Data(uint16_t u16Data) // 0~65535
// {
//     uint8_t au8Data[5] = {0};

//     au8Data[0] = u16Data / 10000 + 0x30;
//     au8Data[1] = (u16Data % 10000) / 1000 + 0x30;
//     au8Data[2] = (u16Data % 1000) / 100 + 0x30;
//     au8Data[3] = (u16Data % 100) / 10 + 0x30;
//     au8Data[4] = u16Data % 10 + 0x30;
//     Uart2_vSend((const char *)au8Data, sizeof(au8Data));
// }

// 自定义的UART格式化打印函数（类似printf）
void Uart2_vprintf(const char *format, ...)
{
    va_list  args;        // 可变参数列表
    char     buffer[256]; // 用于存储格式化后的字符串
    uint32_t len;         // 格式化后的字符串长度

    // 初始化可变参数列表
    va_start(args, format);

    // 将格式化字符串写入缓冲区
    // 使用vsnprintf而非sprintf，更安全（避免缓冲区溢出）
    len = vsnprintf(buffer, sizeof(buffer), format, args);

    // 结束可变参数处理
    va_end(args);

    // 确保不超过缓冲区大小
    if (len >= sizeof(buffer))
    {
        len = sizeof(buffer) - 1; // 截断到缓冲区最大容量
    }

    // 通过UART发送格式化后的字符串
    Uart2_vSend((uint8_t *)buffer, len);
}

void Uart2_vInit(void)
{
    /*Uart config struct*/
    static const UART_Config_t uartCfg =
        {
            115200,
            UART_DATABITS_8,
            UART_PARITY_NONE,
            UART_STOPBITS_1,
            DISABLE,
            40000000,
        };

    /*Config UART1 clock, enable UART1 module*/
    CLK_ModuleSrc(CLK_UART2, CLK_SRC_OSC40M);
    CLK_SetClkDivider(CLK_UART2, CLK_DIV_1);
    SYSCTRL_EnableModule(SYSCTRL_UART2);

    /* Inital UART2 */
    UART_Init(UART2_ID, &uartCfg);

    /*Config UART2 pinmux*/
    PORT_PinmuxConfig(PORT_D, GPIO_5, PTD5_UART2_RX);
    PORT_PinmuxConfig(PORT_C, GPIO_1, PTC1_UART2_TX);

    /*When UART re-open after close, need to check and clear busy status*/
    while (ERR == UART_WaitBusyClear(UART2_ID, 7000))
    {
        (void)UART_ReceiveByte(UART2_ID);
    }

    /*rx config*/
    const UART_FIFOConfig_t fifoCfg =
        {
            ENABLE, ENABLE, ENABLE, UART_TX_FIFO_HALF, UART_RX_FIFO_CHAR_1};
    UART_FIFOConfig(UART2_ID, &fifoCfg);
    UART_InstallCallBackFunc(UART2_ID, UART_INT_RBFI, Uart2_vReadInt);
    UART_IntMask(UART2_ID, UART_INT_RBFI, UNMASK);
    NVIC_EnableIRQ(UART2_IRQn);

    /*regfile init*/
    SYSCTRL_ResetModule(SYSCTRL_REGFILE);
    SYSCTRL_EnableModule(SYSCTRL_REGFILE);
}
/*****************************************************************************
 * End file Uart2.c
 *****************************************************************************/
