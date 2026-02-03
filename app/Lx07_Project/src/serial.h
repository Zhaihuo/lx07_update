/**************************************************************************************************/
/**
 * @file     serial.h
 * @brief
 * @version  V1.0.0
 * @date     December-2023
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/

#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_uart.h"
#include "Z20K11xM_gpio.h"

extern void Uart_Init(void);
extern void uart_send(const char *logBuf, uint32_t logLen);

#endif /* SERIAL_H */