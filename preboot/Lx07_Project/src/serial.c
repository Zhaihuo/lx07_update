/**************************************************************************************************/
/**
 * @file     serial.c
 * @brief
 * @version  V1.0.0
 * @date     December-2023
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/
#include "serial.h"

void Uart_Init(void)
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

    /*Config UART0 clock, enable UART0 module*/
    CLK_ModuleSrc(CLK_UART0, CLK_SRC_OSC40M);
    CLK_SetClkDivider(CLK_UART0, CLK_DIV_1);
    SYSCTRL_EnableModule(SYSCTRL_UART0);

    /* Inital UART0 */
    UART_Init(UART0_ID, &uartCfg);

    /*Config UART0 pinmux*/
    PORT_PinmuxConfig(PORT_A, GPIO_12, PTA12_UART0_RX);
    PORT_PinmuxConfig(PORT_A, GPIO_13, PTA13_UART0_TX);

    /*When UART re-open after close, need to check and clear busy status*/
    while (ERR == UART_WaitBusyClear(UART0_ID, 7000))
    {
        (void)UART_ReceiveByte(UART0_ID);
    }
}

void uart_send(const char *logBuf, uint32_t logLen)
{
    uint32_t i  = 0;
    char     ch = 0;
    for (i = 0; i < logLen; i++)
    {
        ch = *(logBuf + i);
        /* Wait untill FIFO is empty */
        while (RESET == UART_GetLineStatus(UART0_ID, UART_LINESTA_TEMT));

        /* Send data */
        UART_SendByte(UART0_ID, ch);
    }
}

/* Use the printf output to the UART */
int fputc(int ch, FILE *f)
{
    /* Wait untill FIFO is empty */
    while (RESET == UART_GetLineStatus(UART0_ID, UART_LINESTA_TEMT));

    /* Send data */
    UART_SendByte(UART0_ID, (uint8_t)ch);
    return ch;
}
