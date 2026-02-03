/*****************************************************************************
 * @file VedioDisp.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-11-07
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "VedioDisp.h"
#include "i2c.h"
#include "Config.h"
#include "ProductLine.h"
#include "Debounce.h"
#include "can.h"
#include "Touch.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define VEDIODISP_VPG_PICTURE_NUM (4)
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
static uint8_t u8ColorTyp = 1;

uint8_t     VedioDisp_u8FpsHz        = CAN_Write_Fps60HZ;
static bool VedioDisp_boreadFpsHzFlg = false;

static I2cSendData au8SerRGB[] =
    {
        {DEV_SER, 0x01E5, 0x01},
        {DEV_SER, 0x01E6, 0x04},
        {DEV_SER, 0x01E7, 0x00},
        {DEV_SER, 0x01E8, 0x00},
        {DEV_SER, 0x01E9, 0x00},
        {DEV_SER, 0x01EA, 0xff},
        {DEV_SER, 0x01EB, 0xff},
        {DEV_SER, 0x01EC, 0xff},
        {DEV_SER, 0x01ED, 0x80},
        {DEV_SER, 0x01EE, 0x80},
        {DEV_SER, 0x01EF, 0x70}
};
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static void VedioDisp_vVpgColor(void);

/*****************************************************************************
 * function definitions
 *****************************************************************************/
bool VedioDisp_boReadFpsEnd(void)
{
    static bool boInitFlg = true;

    if (boInitFlg)
    {
        boInitFlg = false;

        ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_Fps] = true;
    }
    else
    {}

    return VedioDisp_boreadFpsHzFlg;
}

void VedioDisp_vHandle(void) // 5ms
{
#ifdef TOUCH_NEED_TPCOUNT // SW3作为触摸次数清零按键

#else // SW3作为切换颜色按键
    if (Debounce_boGetValidStu(&Debounce_StSw3))
    {
#ifdef ENABLE_HDMI
#else
        if (!ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_VPGColor])
            ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_VPGColor] = true;

        if (u8ColorTyp < VEDIODISP_VPG_PICTURE_NUM)
        {
            u8ColorTyp++;
        }
        else
        {
            u8ColorTyp = 1;
        }
#endif
    }
    else
    {}
#endif

    VedioDisp_vVpgColor();
}

static void VedioDisp_vVpgColor(void)
{
    if (u8ColorTyp == 1) // red
    {
        au8SerRGB[2u].u8RegData = 0xffu;
        au8SerRGB[3u].u8RegData = 0x00u;
        au8SerRGB[4u].u8RegData = 0x00u;
        au8SerRGB[5u].u8RegData = 0xffu;
        au8SerRGB[6u].u8RegData = 0x00u;
        au8SerRGB[7u].u8RegData = 0x00u;
    }
    else if (u8ColorTyp == 2) // green
    {
        au8SerRGB[2u].u8RegData = 0x00u;
        au8SerRGB[3u].u8RegData = 0xffu;
        au8SerRGB[4u].u8RegData = 0x00u;
        au8SerRGB[5u].u8RegData = 0x00u;
        au8SerRGB[6u].u8RegData = 0xffu;
        au8SerRGB[7u].u8RegData = 0x00u;
    }
    else if (u8ColorTyp == 3) // blue
    {
        au8SerRGB[2u].u8RegData = 0x00u;
        au8SerRGB[3u].u8RegData = 0x00u;
        au8SerRGB[4u].u8RegData = 0xffu;
        au8SerRGB[5u].u8RegData = 0x00u;
        au8SerRGB[6u].u8RegData = 0x00u;
        au8SerRGB[7u].u8RegData = 0xffu;
    }
    else if (u8ColorTyp == VEDIODISP_VPG_PICTURE_NUM) // white
    {
        au8SerRGB[2u].u8RegData = 0xffu;
        au8SerRGB[3u].u8RegData = 0xffu;
        au8SerRGB[4u].u8RegData = 0xffu;
        au8SerRGB[5u].u8RegData = 0xffu;
        au8SerRGB[6u].u8RegData = 0xffu;
        au8SerRGB[7u].u8RegData = 0xffu;
    }
    // else if (u8ColorTyp == 6) // black
    // {
    //     au8SerRGB[2u].u8RegData = 0x00u;
    //     au8SerRGB[3u].u8RegData = 0x00u;
    //     au8SerRGB[4u].u8RegData = 0x00u;
    //     au8SerRGB[5u].u8RegData = 0x00u;
    //     au8SerRGB[6u].u8RegData = 0x00u;
    //     au8SerRGB[7u].u8RegData = 0x00u;
    // }
    // else if (u8ColorTyp == VEDIODISP_VPG_PICTURE_NUM) // checkboard
    // {
    //     au8SerRGB[2u].u8RegData = 0x00u;
    //     au8SerRGB[3u].u8RegData = 0x00u;
    //     au8SerRGB[4u].u8RegData = 0x00u;
    //     au8SerRGB[5u].u8RegData = 0xffu;
    //     au8SerRGB[6u].u8RegData = 0xffu;
    //     au8SerRGB[7u].u8RegData = 0xffu;
    // }

    // if (u8ColorTyp == 5) // ColorBar
    // {
    //     au8SerRGB[0u].u8RegData = 0x02u;
    // }
    // else // checkboard
    // {
    //     au8SerRGB[0u].u8RegData = 0x01u;
    // }

    au8SerRGB[8u].u8RegData  = 0x40u;
    au8SerRGB[9u].u8RegData  = 0x80u;
    au8SerRGB[10u].u8RegData = 0x90u;
}

void VedioDisp_vWriteVpgColor(void) // 配置VPG模式下颜色
{
    static uint8_t u8StepWColorCt = 2;

    if (2 == u8StepWColorCt)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, DEV_SER);
        I2C_Enable(I2C0_ID);

        u8StepWColorCt = 1;
    }
    else if (1 == u8StepWColorCt)
    {
        for (uint8_t u8Index = 0u; u8Index < 11u; u8Index++)
            Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&au8SerRGB[u8Index].u16DstRegAddr, (uint8_t *)&au8SerRGB[u8Index].u8RegData);

        u8StepWColorCt = 0;

        // UART_PRINTF("u8ColorTyp = %d\r\n", u8ColorTyp);
    }
    else if (0 == u8StepWColorCt)
    {
        ProductLine_enCurWorkSts = ProDWork_Idle;

        u8StepWColorCt = 2;
    }
    else
    {}
}

void VedioDisp_vReadFps(void) // 从Eep读Fps
{
    static uint8_t u8StepRdFpsCt = 3;
    uint8_t        u8Addr        = 0x7B;

    if (3 == u8StepRdFpsCt)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x54);
        I2C_Enable(I2C0_ID);
        u8StepRdFpsCt = 2;
    }
    else if (2 == u8StepRdFpsCt)
    {
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&u8Addr));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Fps] = true;

        u8StepRdFpsCt = 1;
    }
    else if (1 == u8StepRdFpsCt)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Fps])
        {
            ProductLine_enCurWorkSts = ProDWork_Idle;
            u8StepRdFpsCt            = 3;

            if ((CAN_Write_Fps60HZ != ProductLine_stI2cRWMsgs.Read_t.au8Data[0]) && (CAN_Write_Fps45HZ != ProductLine_stI2cRWMsgs.Read_t.au8Data[0]))
            {
                VedioDisp_u8FpsHz = CAN_Write_Fps60HZ; // Eep还未写入Fps
            }
            else
            {
                VedioDisp_u8FpsHz = ProductLine_stI2cRWMsgs.Read_t.au8Data[0]; // read data
            }

            VedioDisp_boreadFpsHzFlg = true;
        }
        else
        {}
    }
    else
    {}
}

void VedioDisp_vWriteFps(void)
{
    static uint8_t u8StepWrFpsCt = 5;

    uint8_t au8WriteFps[2] = {0x7B, 0x00}; // 写入地址：0x7B

    if (5 == u8StepWrFpsCt)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x54);
        I2C_Enable(I2C0_ID);
        u8StepWrFpsCt = 4;
    }
    else if (4 == u8StepWrFpsCt)
    {
        au8WriteFps[1] = VedioDisp_u8FpsHz;

        Ex_MstWriteEEP(I2C0_ID, (uint8_t *)(&au8WriteFps[0]), 1);

        u8StepWrFpsCt = 3;

        /*Tx 0x500*/
        memset(stCanRxBuf.data, 0, 8);
        stCanRxBuf.data[1] = VedioDisp_u8FpsHz;
        stCanRxBuf.data[2] = 0x01;
        CAN_Send_Msg(0x500, stCanRxBuf.data);
    }
    else if (3 == u8StepWrFpsCt)
    {
        u8StepWrFpsCt = 2;
    }
    else if (2 == u8StepWrFpsCt)
    {
        u8StepWrFpsCt = 1;
    }
    else if (1 == u8StepWrFpsCt)
    {
        ProductLine_enCurWorkSts = ProDWork_Idle;

        u8StepWrFpsCt = 5;

        NVIC_SystemReset();
    }
    else
    {}
}

void VedioDisp_vCfgVpgColorFromCan(uint8_t u8Cfg)
{
    if (CAN_Write_LastVpgPicture == u8Cfg)
    {
        if (u8ColorTyp <= 1)
        {
            u8ColorTyp = VEDIODISP_VPG_PICTURE_NUM;
        }
        else
        {
            u8ColorTyp--;
        }
    }
    else if (CAN_Write_NextVpgPicture == u8Cfg)
    {
        if (u8ColorTyp < VEDIODISP_VPG_PICTURE_NUM)
        {
            u8ColorTyp++;
        }
        else
        {
            u8ColorTyp = 1;
        }
    }
    else if (CAN_Write_VpgRed == u8Cfg)
    {
        u8ColorTyp = 1; // red
    }
    else if (CAN_Write_VpgGreen == u8Cfg)
    {
        u8ColorTyp = 2; // green
    }
    else if (CAN_Write_VpgBlue == u8Cfg)
    {
        u8ColorTyp = 3; // blue
    }
    else if (CAN_Write_VpgWhite == u8Cfg)
    {
        u8ColorTyp = 4; // white
    }
    else
    {}
}

void VedioDisp_vCfgFpsFromCan(uint8_t u8Cfg)
{
    if (CAN_Write_Fps60HZ == u8Cfg)
    {
        VedioDisp_u8FpsHz = CAN_Write_Fps60HZ;
    }
    else if (CAN_Write_Fps45HZ == u8Cfg)
    {
        VedioDisp_u8FpsHz = CAN_Write_Fps45HZ;
    }
    else
    {}
}
/*****************************************************************************
 * End file VedioDisp.c
 *****************************************************************************/
