/*****************************************************************************
 * @file Touch.c
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
#include "Touch.h"
#include "stdint.h"
#include "i2c.h"
#include "ProductLine.h"
#include "focaltech_core.h"
#include "Debounce.h"
#include "uart.h"
#include "Config.h"
#include "can.h"
#include "VedioDisp.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/

/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
static uint32_t Touch_u32TpCountOld = 0xFFFFFFFF;
uint32_t        Touch_u32TpCount    = 0;
bool            Touch_boMisTpIntFlg = false;
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static void Touch_vRecordTpCount(void);
static void Touch_vMisTpTest(void);
/*****************************************************************************
 * function definitions
 *****************************************************************************/
void Touch_vInit(void)
{
#ifdef TOUCH_NEED_TPCOUNT
    ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_TpCount] = true;
#endif

#ifdef TOUCH_NEED_TPCOUNT
    Debounce_vInit(&Debounce_StTpTrigger, MAIN_TIME_MS(80));
#else
    Debounce_vInit(&Debounce_StTpTrigger, 0);
#endif
    Debounce_vInit(&Debounce_StSw3, MAIN_TIME_MS(200)); // 清除触摸次数，3s内按两次
}

void Touch_vHandle(void) // 5ms
{
    Debounce_vCheck(&Debounce_StTpTrigger);
    Debounce_vCheck(&Debounce_StSw3);

    Touch_vMisTpTest();

    if (Debounce_boGetValidStu(&Debounce_StTpTrigger))
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_TpCoord])
            ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_TpCoord] = true;
    }
    else
    {}

#ifdef TOUCH_NEED_TPCOUNT
    Touch_vRecordTpCount();
#endif
}

static void Touch_vRecordTpCount(void)
{
    static uint8_t  u8TpClearCnt   = 0;
    static uint16_t u16TpClearTime = 0;

    uint8_t au8Uart0TxTpCount[] = {0x74, 0x39, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};

    if (Debounce_boGetValidStu(&Debounce_StSw3))
    {
        u8TpClearCnt++;
        if (u8TpClearCnt >= 2)
        {
            u16TpClearTime   = 0;
            u8TpClearCnt     = 0;
            Touch_u32TpCount = 0; // 清除触摸次数
        }
        else
        {}
    }
    else
    {}

    if (u8TpClearCnt > 0)
    {
        u16TpClearTime++;
        if (u16TpClearTime >= MAIN_TIME_MS(2000))
        {
            u16TpClearTime = 0;
            u8TpClearCnt   = 0;
        }
        else
        {}
    }
    else
    {}

    if (Touch_u32TpCountOld != Touch_u32TpCount)
    {
        Touch_u32TpCountOld = Touch_u32TpCount;

        if (!ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_TpCount])
            ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_TpCount] = true;

        au8Uart0TxTpCount[8]  = 0x30 + (Touch_u32TpCount / 100000);
        au8Uart0TxTpCount[9]  = 0x30 + (Touch_u32TpCount % 100000) / 10000;
        au8Uart0TxTpCount[10] = 0x30 + (Touch_u32TpCount % 10000) / 1000;
        au8Uart0TxTpCount[11] = 0x30 + (Touch_u32TpCount % 1000) / 100;
        au8Uart0TxTpCount[12] = 0x30 + (Touch_u32TpCount % 100) / 10;
        au8Uart0TxTpCount[13] = 0x30 + Touch_u32TpCount % 10;

        Uart_Transmit(au8Uart0TxTpCount, sizeof(au8Uart0TxTpCount));
    }
    else
    {}
}

void Touch_vReadTpCoord(void) // 读触摸坐标
{
    static uint8_t u8StepTpCoord = 2;

    uint8_t u8Addr = 0x01;

    uint8_t au8CoordX[] = {0x6E, 0x30, 0x2E, 0x76, 0x61, 0x6C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};
    uint8_t au8CoordY[] = {0x6E, 0x31, 0x2E, 0x76, 0x61, 0x6C, 0x3D, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

    if (2 == u8StepTpCoord)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x38);
        I2C_Enable(I2C0_ID);
        u8StepTpCoord = 1;
    }
    else if (1 == u8StepTpCoord)
    {
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&u8Addr));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_TpCoord] = true;

        u8StepTpCoord = 0;
        // Touch_u32TpCount++;
    }
    else if (0 == u8StepTpCoord)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_TpCoord])
        {
            u8StepTpCoord            = 2;
            ProductLine_enCurWorkSts = ProDWork_Idle;

            struct fts_ts_event events;

            events.x = ((ProductLine_stI2cRWMsgs.Read_t.au8Data[2] & 0x0F) << 8) + ProductLine_stI2cRWMsgs.Read_t.au8Data[2 + 1];
            events.y = ((ProductLine_stI2cRWMsgs.Read_t.au8Data[2 + 2] & 0x0F) << 8) + ProductLine_stI2cRWMsgs.Read_t.au8Data[2 + 3];

            // UART_PRINTF("x = %d, y = %d\r\n", events.x, events.y);

            au8CoordX[7]  = 0x30 + events.x / 1000;
            au8CoordX[8]  = 0x30 + (events.x % 1000) / 100;
            au8CoordX[9]  = 0x30 + (events.x % 100) / 10;
            au8CoordX[10] = 0x30 + events.x % 10;

            au8CoordY[7] = 0x30 + events.y / 100;
            au8CoordY[8] = 0x30 + (events.y % 100) / 10;
            au8CoordY[9] = 0x30 + events.y % 10;

            Uart_Transmit(au8CoordX, sizeof(au8CoordX));
            Uart_Transmit(au8CoordY, sizeof(au8CoordY));
        }
        else
        {}
    }
    else
    {}
}

void Touch_vReadTpCount(void) // 读触摸次数
{
    static uint8_t u8StepTpCt          = 3;
    uint8_t        u8Addr              = 0x7C;
    uint8_t        au8Uart0TxTpCount[] = {0x74, 0x39, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};

    if (3 == u8StepTpCt)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x54);
        I2C_Enable(I2C0_ID);
        u8StepTpCt = 2;
    }
    else if (2 == u8StepTpCt)
    {
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&u8Addr));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_TpCount] = true;

        u8StepTpCt = 1;
    }
    else if (1 == u8StepTpCt)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_TpCount])
        {
            ProductLine_enCurWorkSts = ProDWork_Idle;
            u8StepTpCt               = 3;

            Touch_u32TpCount = (uint8_t)(ProductLine_stI2cRWMsgs.Read_t.au8Data[0] << 24u);
            Touch_u32TpCount = (uint8_t)(ProductLine_stI2cRWMsgs.Read_t.au8Data[1] << 16u);
            Touch_u32TpCount = (uint8_t)(ProductLine_stI2cRWMsgs.Read_t.au8Data[2] << 8u);
            Touch_u32TpCount = (uint8_t)ProductLine_stI2cRWMsgs.Read_t.au8Data[3];

            au8Uart0TxTpCount[8]  = 0x30 + (Touch_u32TpCount / 100000);
            au8Uart0TxTpCount[9]  = 0x30 + (Touch_u32TpCount % 100000) / 10000;
            au8Uart0TxTpCount[10] = 0x30 + (Touch_u32TpCount % 10000) / 1000;
            au8Uart0TxTpCount[11] = 0x30 + (Touch_u32TpCount % 1000) / 100;
            au8Uart0TxTpCount[12] = 0x30 + (Touch_u32TpCount % 100) / 10;
            au8Uart0TxTpCount[13] = 0x30 + Touch_u32TpCount % 10;

            Uart_Transmit(au8Uart0TxTpCount, sizeof(au8Uart0TxTpCount));
        }
        else
        {}
    }
    else
    {}
}

void Touch_vWriteTpCount(void) // 写入触摸次数
{
    static uint8_t u8StepWTpCt = 2;

    uint8_t au8WriteTpCt[5] = {0x7C, 0x01, 0x01, 0x01, 0x01}; // 写入地址：0x7C~0x7F

    if (2 == u8StepWTpCt)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x54);
        I2C_Enable(I2C0_ID);
        u8StepWTpCt = 1;
    }
    else if (1 == u8StepWTpCt)
    {
        au8WriteTpCt[4] = (uint8_t)Touch_u32TpCount;
        au8WriteTpCt[3] = (uint8_t)(Touch_u32TpCount >> 8u);
        au8WriteTpCt[2] = (uint8_t)(Touch_u32TpCount >> 16u);
        au8WriteTpCt[1] = (uint8_t)(Touch_u32TpCount >> 24u);

        Ex_MstWriteEEP(I2C0_ID, (uint8_t *)(&au8WriteTpCt[0]), 4);
        ProductLine_enCurWorkSts = ProDWork_Idle;

        u8StepWTpCt = 2;
    }
    else
    {}
}

void Touch_vMisTpTest(void)
{
    static uint16_t u16TouchCnt = 1;

    if (Touch_boMisTpIntFlg)
    {
        u16TouchCnt++;

        if ((u16TouchCnt % MAIN_TIME_MS(500)) == 0)
        {
            /*Tx 0x500*/
            memset(stCanRxBuf.data, 0, 8);
            stCanRxBuf.data[3] = 0x01;
            CAN_Send_Msg(0x500, stCanRxBuf.data);
        }
        else
        {}

        if (u16TouchCnt > MAIN_TIME_MS(1500))
        {
            u16TouchCnt         = 0;
            Touch_boMisTpIntFlg = false;
        }
        else
        {}
    }
    else
    {
        u16TouchCnt = 0;
    }
}
/*****************************************************************************
 * End file Touch.c
 *****************************************************************************/
