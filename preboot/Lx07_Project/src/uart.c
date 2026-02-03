
#include "uart.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_uart.h"
#include "string.h"

uint8_t u8UartRecvIndex     = 0u;
uint8_t u8UartDataArray[17] = {0u};
uint8_t u8UartCheckData     = 0;
uint8_t u8ClearUart[17]     = {0};

void UartRecvInt(void)
{
    uint8_t u8Data = 0u;
    /* receive data */
    u8Data = UART_ReceiveByte(UART0_ID);

    // if (u8InitFlag == 8)
    if (false)
    {
        if (u8UartRecvIndex == 0)
        {
            memcpy(&u8UartDataArray[0u], &u8ClearUart[0u], 17u);
            if (u8Data == 0xAA || u8Data == 0xBB || u8Data == 0xCC || u8Data == 0xDD)
            {
                u8UartCheckData = u8Data;
                u8UartRecvIndex += 1;
            }
            else
            {
                u8UartRecvIndex = 0;
            }
        }
        else
        {
            u8UartDataArray[u8UartRecvIndex - 1] = u8Data;
            u8UartRecvIndex++;
        }
    }
}

void Uart0_Init(void)
{
    /*Uart config struct*/
    static const UART_Config_t uartCfg =
        {
            9600,
            UART_DATABITS_8,
            UART_PARITY_NONE,
            UART_STOPBITS_1,
            DISABLE,
            40000000,
        };

    /*Config UART1 clock, enable UART1 module*/
    CLK_ModuleSrc(CLK_UART0, CLK_SRC_OSC40M);
    CLK_SetClkDivider(CLK_UART0, CLK_DIV_1);
    SYSCTRL_EnableModule(SYSCTRL_UART0);

    /*Config PortC clock, enable PortC module*/
    CLK_ModuleSrc(CLK_PORTC, CLK_SRC_OSC40M);
    SYSCTRL_EnableModule(SYSCTRL_PORTC);

    /*When UART re-open after close, need to check and clear busy status*/
    while (ERR == UART_WaitBusyClear(UART0_ID, 7000))
    {
        (void)UART_ReceiveByte(UART0_ID);
    }

    /* Inital UART1 */
    UART_Init(UART0_ID, &uartCfg);

    /* fifo config */
    const UART_FIFOConfig_t fifoCfg =
        {
            ENABLE, ENABLE, ENABLE, UART_TX_FIFO_HALF, UART_RX_FIFO_CHAR_1};

    UART_FIFOConfig(UART0_ID, &fifoCfg);

    /*Config UART1 pinmux*/
    PORT_PinmuxConfig(PORT_D, GPIO_16, PTD16_UART0_TX);
    PORT_PinmuxConfig(PORT_D, GPIO_15, PTD15_UART0_RX);

    /* callback function */
    UART_InstallCallBackFunc(UART0_ID, UART_INT_RBFI, UartRecvInt);

    /* enable received data available interrupt  */
    UART_IntMask(UART0_ID, UART_INT_RBFI, UNMASK);

    /* enable IRQ */
    NVIC_EnableIRQ(UART0_IRQn);
}

void Uart_Transmit(uint8_t *logBuf, uint32_t logLen)
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