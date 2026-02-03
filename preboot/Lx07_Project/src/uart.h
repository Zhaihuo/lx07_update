
#ifndef UART_H
#define UART_H

#include "stdint.h"

extern uint8_t u8UartCheckData;
extern uint8_t u8UartDataArray[17];
void           Uart0_Init(void);
void           Uart_Transmit(uint8_t *logBuf, uint32_t logLen);

#endif /* UART_H */