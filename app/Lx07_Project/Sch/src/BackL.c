/*****************************************************************************
 * @file BackL.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-11-06
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "BackL.h"
#include "stdint.h"
#include "i2c.h"
#include "Config.h"
#include "ProductLine.h"
#include "uart.h"
#include "Debounce.h"
#include "can.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
// #define BACKL_REG_MAX (0x3FFF - 3)
#define BACKL_REG_MAX (0x2666) // 60%
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
static uint16_t u16Level = BACKL_REG_MAX;
static uint8_t  Level_H, Level_L = 0;
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/

/*****************************************************************************
 * function definitions
 *****************************************************************************/
void BackL_vInit(void)
{
    Debounce_vInit(&Debounce_StSw1, MAIN_TIME_MS(200)); // down bkl 10%
    Debounce_vInit(&Debounce_StSw2, MAIN_TIME_MS(200)); // up bkl 10%
}

void BackL_vHandle(void)
{
    Debounce_vCheck(&Debounce_StSw1);
    Debounce_vCheck(&Debounce_StSw2);

    if (Debounce_boGetValidStu(&Debounce_StSw1))
    {
        if (!ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl])
            ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl] = true;

        if (u16Level == 0)
        {
            u16Level = BACKL_REG_MAX;
        }
        else
        {
            if (u16Level < BACKL_REG_MAX / 10)
            {
                u16Level = 0;
            }
            else
            {
                u16Level -= (BACKL_REG_MAX / 10);
            }
        }
    }
    else
    {}

    if (Debounce_boGetValidStu(&Debounce_StSw2))
    {
        if (!ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl])
            ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl] = true;

        if (u16Level >= BACKL_REG_MAX)
        {
            u16Level = 0;
        }
        else
        {
            u16Level += (BACKL_REG_MAX / 10);

            if (u16Level >= BACKL_REG_MAX)
            {
                u16Level = BACKL_REG_MAX;
            }
            else
            {}
        }
    }
    else
    {}
}

void BackL_vReadLevel(void)
{
    static uint8_t u8StepReadBkl = 5;
    static uint8_t u8BklLel_H, u8BklLel_L = 0;

    uint16_t u16BklAddr, u16BklRegVal = 0;
    float    flPercent = 0.0;
    uint8_t  u8Percnet = 0;

    uint8_t au8UartTxData[] = {0x6E, 0x32, 0x2E, 0x76, 0x61, 0x6C, 0x3D, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

    if (5 == u8StepReadBkl)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, DEV_BKL);
        I2C_Enable(I2C0_ID);

        u8StepReadBkl = 4;
    }
    else if (4 == u8StepReadBkl)
    {
        u16BklAddr = 0x0005; // addr_h
        Ex_MstReadBuffersss(I2C0_ID, (uint8_t *)(&u16BklAddr));

        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Bkl] = true;

        u8StepReadBkl = 3;
    }
    else if (3 == u8StepReadBkl)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Bkl])
        {
            u8BklLel_H = ProductLine_stI2cRWMsgs.Read_t.au8Data[0];

            u8StepReadBkl = 2;
        }
        else
        {}
    }
    else if (2 == u8StepReadBkl)
    {
        u16BklAddr = 0x0006; // addr_l
        Ex_MstReadBuffersss(I2C0_ID, (uint8_t *)(&u16BklAddr));

        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Bkl] = true;

        u8StepReadBkl = 1;
    }
    else if (1 == u8StepReadBkl)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Bkl])
        {
            u8BklLel_L = ProductLine_stI2cRWMsgs.Read_t.au8Data[0];

            u8StepReadBkl            = 5;
            ProductLine_enCurWorkSts = ProDWork_Idle;

            u16BklRegVal = (u8BklLel_H << 8u) | u8BklLel_L;
            flPercent    = u16BklRegVal * 1.0 / BACKL_REG_MAX;
            u8Percnet    = (flPercent + 0.005) * 100;

            if (u8Percnet == 100)
            {
                au8UartTxData[7] = 0x30 + u8Percnet / 100;
                au8UartTxData[8] = 0x30 + (u8Percnet % 100) / 10;
                au8UartTxData[9] = 0x30 + u8Percnet % 10;
            }
            else
            {
                au8UartTxData[7] = 0x30;
                au8UartTxData[8] = 0x30 + u8Percnet / 10;
                au8UartTxData[9] = 0x30 + u8Percnet % 10;
            }

            Uart_Transmit(au8UartTxData, sizeof(au8UartTxData));
        }
        else
        {}
    }
    else
    {}
}

void BackL_vWriteLevel(void)
{
    uint8_t u8Addr_H, u8Addr_L, u8BklPercent = 0;

    uint8_t au8Uart0TxBkl[] = {0x6E, 0x32, 0x2E, 0x76, 0x61, 0x6C, 0x3D, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

    static uint8_t u8StepWBkl = 2;

    if (2 == u8StepWBkl)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, DEV_BKL);
        I2C_Enable(I2C0_ID);

        u8StepWBkl = 1;
    }
    else if (1 == u8StepWBkl)
    {
        Level_L = u16Level;
        Level_H = u16Level >> 8u;

        u8Addr_H = 0x05;
        Lx07_MstWriteByte(I2C0_ID, (uint8_t *)&u8Addr_H, (uint8_t *)&Level_H);
        u8Addr_L = 0x06;
        Lx07_MstWriteByte(I2C0_ID, (uint8_t *)&u8Addr_L, (uint8_t *)&Level_L);

        ProductLine_enCurWorkSts = ProDWork_Idle;
        u8StepWBkl               = 2;

        float Percent = u16Level * 1.0 / BACKL_REG_MAX;

        // UART_PRINTF("PercentLevel = %.2f\r\n", Percent);

        u8BklPercent = (Percent + 0.005) * 100;

        if (u8BklPercent == 100)
        {
            au8Uart0TxBkl[7] = 0x30 + u8BklPercent / 100;
            au8Uart0TxBkl[8] = 0x30 + (u8BklPercent % 100) / 10;
            au8Uart0TxBkl[9] = 0x30 + u8BklPercent % 10;
        }
        else
        {
            au8Uart0TxBkl[7] = 0x30;
            au8Uart0TxBkl[8] = 0x30 + u8BklPercent / 10;
            au8Uart0TxBkl[9] = 0x30 + u8BklPercent % 10;
        }

        Uart_Transmit(au8Uart0TxBkl, sizeof(au8Uart0TxBkl));
    }
}

void BackL_vWriteLevFromCan(uint8_t u8Percent)
{
    if (u8Percent > 100)
    {
        /*Invalid value*/
    }
    else
    {
        u16Level = BACKL_REG_MAX * 1.0 / 100 * u8Percent;
    }
}

void BackL_vDireating(uint8_t u8Temp)
{
    float flBklPercent = 0;

    if (u8Temp < 65)
    {
        // LCD最大亮度可以是100%，不做处理
    }
    else if (u8Temp > 85)
    {
        // 10%
        flBklPercent = 10;
        if (u16Level > (BACKL_REG_MAX * 1.0 / 100 * flBklPercent))
            u16Level = BACKL_REG_MAX * 1.0 / 100 * flBklPercent;
        if (!ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl])
            ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl] = true;
    }
    else
    {
        flBklPercent = 392.5 - 4.5 * u8Temp;

        if (u16Level > (BACKL_REG_MAX * 1.0 / 100 * flBklPercent))
            u16Level = BACKL_REG_MAX * 1.0 / 100 * flBklPercent;
        if (!ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl])
            ProductLine_stI2cRWMsgs.Write_t.aboWFlg[ProD_I2c_Write_Bkl] = true;
    }
}
/*****************************************************************************
 * End file BackL.c
 *****************************************************************************/
