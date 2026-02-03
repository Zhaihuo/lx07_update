/**************************************************************************************************/
/**
 * @file     can.c
 * @brief
 * @version  V1.0.0
 * @date     June-2024
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/

#include "can.h"
#include "Eeprom.h"
#include <stddef.h>
#include "ProductLine.h"
#include "Eeprom.h"
#include "BackL.h"
#include "VedioDisp.h"
#include "Config.h"
#include "Lx07.h"
#include "Touch.h"
#include "VedioDisp.h"

#define CAN_EEP_ReadAddr_OFFSET (0x20)

CAN_MsgBuf_t stCanRxBuf = {0};

static uint8_t Can_au8CanTx502[8] = {0};

static CAN_Config_t Can_cfg =
    {
        .mbMaxNum          = 16,
        .rxFifoEn          = ENABLE,
        .rxFifoIdFilterNum = CAN_RX_FIFO_ID_FILTERS_8,
        .mode              = CAN_MODE_NORMAL,
        .fdEn              = DISABLE,
        .bitTiming =
            {
                        .propSeg    = 4,
                        .phaseSeg1  = 3,
                        .phaseSeg2  = 2,
                        .preDivider = 4,
                        .rJumpwidth = 1},
        .payload0 = CAN_PAYLOAD_SIZE_8,
        .payload1 = CAN_PAYLOAD_SIZE_8,

        .bitTimingFdData =
            {
                        .propSeg    = 4,
                        .phaseSeg1  = 3,
                        .phaseSeg2  = 2,
                        .preDivider = 1,
                        .rJumpwidth = 1},
};

static CAN_MessageInfo_t tx_info =
    {
        .idType     = CAN_MSG_ID_STD, /*msg type: standard frame*/
        .dataLen    = 8,              /*date length*/
        .remoteFlag = RESET,          /*isn't remote frame */
        .fdEn       = DISABLE,        /*FD enabled*/
        .fdPadding  = 0,              /*Use zeros for FD padding*/
        .brsEn      = DISABLE         /*Bit rate switch enabled*/
};

CAN_IdFilter_t fifo_id_filter[8] = {
    {RESET, RESET, 0x730},
    {RESET, RESET, 0x731},
    {RESET, RESET, 0x732},
    {RESET, RESET, 0x733},
    {RESET, RESET, 0x734},
    {RESET, RESET, 0x735},
    {RESET, RESET, 0x736},
    {RESET, RESET, 0x737},
};

void RxFifo_Callback(void)
{
    CAN_ReadRxFifo(CAN_ID_0, &stCanRxBuf);

    int Rtval = -1;

    uint8_t u8CanData0 = stCanRxBuf.data[0];

    if (stCanRxBuf.msgId == 0x730)
    {
        switch (u8CanData0)
        {
            case CAN_READ_Bkl: // i2c read Bkl
                ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_Bkl] = true;
                break;
            case CAN_READ_LcdTemp: // i2c read lcd temp
                ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_LcdTemp] = true;
                ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_LcdTemp]  = true;
                break;
            case CAN_READ_PcbTemp: // i2c read Pcb temp
                ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_PcbTemp] = true;
                ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_PcbTemp]  = true;
                break;
            // case CAN_READ_Batt:
            //     ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_Batt] = true;
            //     ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_Batt]  = true;
            //     break;
            default:
                break;
        }
    }
    else if (stCanRxBuf.msgId == 0x731)
    {
        switch (u8CanData0)
        {
            case CAN_Write_Bkl: // Write Bkl
                ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl] = true;
                BackL_vWriteLevFromCan(stCanRxBuf.data[1]);
                break;
            case CAN_Write_Fps60HZ:
                ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Fps] = true;
                VedioDisp_vCfgFpsFromCan(u8CanData0);
                break;
            case CAN_Write_Fps45HZ:
                ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Fps] = true;
                VedioDisp_vCfgFpsFromCan(u8CanData0);
                break;
            default:
#ifdef ENABLE_HDMI
#else
                if (((u8CanData0 >= CAN_Write_LastVpgPicture) && (u8CanData0 <= CAN_Write_NextVpgPicture)) || ((u8CanData0 >= CAN_Write_VpgRed) && (u8CanData0 <= CAN_Write_VpgWhite)))
                {
                    ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_VPGColor] = true;
                    VedioDisp_vCfgVpgColorFromCan(u8CanData0);
                }
                else
                {}
#endif
                break;
        }
    }
    else
    {}
}

void CAN_Send_Msg(uint32_t msgId, const uint8_t *msgData)
{
    static uint8_t u8MbId = 10;

    if (u8MbId <= 14)
    {
        u8MbId++;
    }
    else
    {
        u8MbId = 10;
    }

    if (0x500 == msgId)
    {
        stCanRxBuf.data[0] = DEVICE_ID;
    }
    else
    {}

    CAN_Send(CAN_ID_0, u8MbId, &tx_info, msgId, msgData);
}

void CANConfig_Init(void)
{
    /* Set module clock */
    CLK_ModuleSrc(CLK_CAN0, CLK_SRC_OSC40M);
    CLK_SetClkDivider(CLK_CAN0, CLK_DIV_2);

    /* Enable module */
    SYSCTRL_ResetModule(SYSCTRL_CAN0);
    SYSCTRL_EnableModule(SYSCTRL_CAN0);
    /* Init CAN */
    CAN_Init(CAN_ID_0, &Can_cfg);

    PORT_PinmuxConfig(PORT_A, GPIO_11, PTA11_CAN0_TX);
    PORT_PinmuxConfig(PORT_A, GPIO_10, PTA10_CAN0_RX);

    CAN_InstallCallBackFunc(CAN_ID_0, CAN_INT_RXFIFO_FRAME, RxFifo_Callback);
    CAN_IntMask(CAN_ID_0, CAN_INT_RXFIFO_FRAME, UNMASK, 0, 0);

    NVIC_SetPriority(CAN0_IRQn, 0x1);
    NVIC_EnableIRQ(CAN0_IRQn);

    CAN_ConfigRxFifo(CAN_ID_0, CAN_RX_FIFO_ID_FORMAT_A, fifo_id_filter);

    /* Global Mask type */
    CAN_SetRxMaskType(CAN_ID_0, CAN_RX_MASK_GLOBAL);

    CAN_SetRxFifoGlobalMask(CAN_ID_0, 0xfffffffe);
}
