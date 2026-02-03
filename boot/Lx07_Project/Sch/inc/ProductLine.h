/*****************************************************************************
 * @file ProductLine.h
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-11-05
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/
#ifndef PRODUCTLINE_H
#define PRODUCTLINE_H

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
/*****************************************************************************
 * Global macros
 *****************************************************************************/

/*****************************************************************************
 * Global data types
 *****************************************************************************/
typedef enum
{
    ProDWork_Read_Bkl,
    ProDWork_Read_LcdTemp,
    ProDWork_Read_PcbTemp,
    ProDWork_Read_Batt,
    ProDWork_Read_AdHw,
    // 个人定义
    ProDWork_Read_TpCoord,
    ProDWork_Read_TpCount,
    ProDWork_Read_Fps,

    ProDWork_Write_TpCount,
    ProDWork_Write_Bkl,
    ProDWork_Write_VPGColor,
    ProDWork_Write_Fps,

    ProDWork_Idle = 0xff,
} ProductLine_enWorkSts;

typedef enum
{
    ProD_I2c_Read_Bkl,
    ProD_I2c_Read_LcdTemp,
    ProD_I2c_Read_PcbTemp,
    ProD_I2c_Read_Batt,
    ProD_I2c_Read_AdHw,
    // 个人定义
    ProD_I2c_Read_TpCoord,
    ProD_I2c_Read_TpCount,
    ProD_I2c_Read_Fps,

    ProD_I2c_Read_Max
} ProductLine_enI2cReadTyp;

typedef enum
{
    ProD_I2c_Write_TpCount,
    ProD_I2c_Write_Bkl,
    ProD_I2c_Write_VPGColor,
    ProD_I2c_Write_Fps,

    ProD_I2c_Write_Max
} ProductLine_enI2cWriteTyp;

typedef struct
{
    struct
    {
        ProductLine_enI2cReadTyp enTyp;

        uint8_t u8Count; // i2c读数据计数
        uint8_t au8Data[8];

        bool aboStrtFlg[ProD_I2c_Read_Max];
        bool aboEndFlg[ProD_I2c_Read_Max];

        bool aboHaveTx[ProD_I2c_Read_Max]; // 是否需要外发

    } Read_t;

    struct
    {
        ProductLine_enI2cWriteTyp enTyp;

        bool aboWFlg[ProD_I2c_Write_Max];

    } Write_t;

} ProductLine_stI2cRW;
/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
extern ProductLine_enWorkSts ProductLine_enCurWorkSts;
extern ProductLine_stI2cRW   ProductLine_stI2cRWMsgs;
/*****************************************************************************
 * Global function prototypes
 *****************************************************************************/
void ProductLine_vHandle(void);
void ProductLine_vInit(void);

void ProductLine_vI2cRecvHandle(void);
void ProductLine_vI2cStopHandle(void);
#endif
/*****************************************************************************
 * End file PRODUCTLINE_H
 *****************************************************************************/
