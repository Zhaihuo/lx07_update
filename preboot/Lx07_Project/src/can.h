/**************************************************************************************************/
/**
 * @file     can.h
 * @brief
 * @version  V1.0.0
 * @date     June-2024
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/

#ifndef CAN_H
#define CAN_H

#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_can.h"

extern CAN_MsgBuf_t stCanRxBuf;

extern void CANConfig_Init(void);
extern void CAN_Send_Msg(uint32_t msgId, const uint8_t *msgData);
#endif /* CAN_H */
