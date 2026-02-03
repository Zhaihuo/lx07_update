/*****************************************************************************
 * @file ProductLine.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-11-05
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "ProductLine.h"
#include "Eeprom.h"
#include "i2c.h"

#include "Adc.h"
#include "BackL.h"
#include "Touch.h"
#include "VedioDisp.h"
#include "uart.h"
#include "can.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define PRODLINE_READ_TP_SIZE (6) // 读取触摸信息长度
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
ProductLine_enWorkSts ProductLine_enCurWorkSts = ProDWork_Idle;

ProductLine_stI2cRW ProductLine_stI2cRWMsgs = {0};

static uint8_t au8ProDReadDataSize[ProD_I2c_Read_Max] = {

    1,                     // ProD_I2c_Read_Bkl,
    2,                     // ProD_I2c_Read_LcdTemp,
    2,                     // ProD_I2c_Read_PcbTemp,
    2,                     // ProD_I2c_Read_Batt,
    2,                     // ProD_I2c_Read_AdHw,
    PRODLINE_READ_TP_SIZE, // ProD_I2c_Read_TpCoord
    4,                     // ProD_I2c_Read_TpCount
    1,                     // ProD_I2c_Read_Fps
};
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static void ProductLine_vWorkSts(void);
/*****************************************************************************
 * function definitions
 *****************************************************************************/
void ProductLine_vInit(void)
{
    Touch_vInit();
    Adc_vInit();
    BackL_vInit();

    // ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_EEP] = true;
}

void ProductLine_vHandle(void)
{
    VedioDisp_vHandle();
    Touch_vHandle();
    Adc_vHandle();
    BackL_vHandle();
    ProductLine_vWorkSts();
}

static void ProductLine_vWorkSts(void)
{
    ProductLine_enI2cReadTyp  enReadTpy  = 0;
    ProductLine_enI2cWriteTyp enWriteTpy = 0;

    if (ProDWork_Idle == ProductLine_enCurWorkSts && boI2c0IsIdle)
    {
        for (enReadTpy = 0; enReadTpy < ProD_I2c_Read_Max; enReadTpy++) // 查询是否有产线读取指令
        {
            if (ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[enReadTpy])
            {
                ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[enReadTpy] = false;
                ProductLine_stI2cRWMsgs.Read_t.enTyp                 = enReadTpy;
                ProductLine_enCurWorkSts                             = (ProductLine_enWorkSts)enReadTpy;
                break;
            }
            else
            {}
        }

        if (ProD_I2c_Read_Max == enReadTpy) // 没有产线读取指令
        {
            for (enWriteTpy = 0; enWriteTpy < ProD_I2c_Write_Max; enWriteTpy++) // 查询是否有产线写入指令
            {
                if (ProductLine_stI2cRWMsgs.Write_t.aboWFlg[enWriteTpy])
                {
                    ProductLine_stI2cRWMsgs.Write_t.aboWFlg[enWriteTpy] = false;
                    ProductLine_stI2cRWMsgs.Write_t.enTyp               = enWriteTpy;
                    ProductLine_enCurWorkSts                            = (ProductLine_enWorkSts)(ProD_I2c_Read_Max + enWriteTpy);
                    break;
                }
                else
                {}
            }
        }
        else
        {}
    }
    else
    {}

    // 执行i2c读取或者写入任务

    switch (ProductLine_enCurWorkSts)
    {
        /*I2C Read*/
        case ProDWork_Read_Bkl:
            BackL_vReadLevel();
            break;
        case ProDWork_Read_LcdTemp:
            Adc_vReadLcdTemp();
            break;
        case ProDWork_Read_PcbTemp:
            Adc_vReadPcbTemp();
            break;
        case ProDWork_Read_Batt:
            Adc_vReadBatt();
            break;
        case ProDWork_Read_AdHw:
            Adc_vReadHw();
            break;
            // 个人定义
        case ProDWork_Read_TpCoord:
            Touch_vReadTpCoord();
            break;
        case ProDWork_Read_TpCount:
            Touch_vReadTpCount();
            break;
        case ProDWork_Read_Fps:
            VedioDisp_vReadFps();
            break;
        /*I2C Write*/
        case ProDWork_Write_TpCount:
            Touch_vWriteTpCount();
            break;
        case ProDWork_Write_Bkl:
            BackL_vWriteLevel();
            break;
        case ProDWork_Write_VPGColor:
            VedioDisp_vWriteVpgColor();
            break;
        case ProDWork_Write_Fps:
            VedioDisp_vWriteFps();
            break;
        default:
            break;
    }
}

void ProductLine_vI2cRecvHandle(void) // 在i2c中断接收数据
{
    if (ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProductLine_stI2cRWMsgs.Read_t.enTyp])
    {
        ProductLine_stI2cRWMsgs.Read_t.au8Data[ProductLine_stI2cRWMsgs.Read_t.u8Count] = I2C_ReceiveByte(I2C0_ID);
        ProductLine_stI2cRWMsgs.Read_t.u8Count++;

        if (ProductLine_stI2cRWMsgs.Read_t.u8Count < au8ProDReadDataSize[ProductLine_stI2cRWMsgs.Read_t.enTyp])
        {
            MstRecvByType(I2C0_ID, 1, I2C_RESTART_AND_STOP_DISABLE);
        }
        else
        {
            MstRecvByType(I2C0_ID, 1, I2C_STOP_EN);
        }
    }
}

void ProductLine_vI2cStopHandle(void) // 在i2c读取停止中断清除标志位
{
    if ((ProductLine_stI2cRWMsgs.Read_t.u8Count >= au8ProDReadDataSize[ProductLine_stI2cRWMsgs.Read_t.enTyp]) && ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProductLine_stI2cRWMsgs.Read_t.enTyp])
    {
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProductLine_stI2cRWMsgs.Read_t.enTyp] = false;
        ProductLine_stI2cRWMsgs.Read_t.u8Count                                         = 0;
    }
}
/*****************************************************************************
 * End file ProductLine.c
 *****************************************************************************/
