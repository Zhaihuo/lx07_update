/*****************************************************************************
 * @file Config.h
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-11-04
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "Z20K11xM_drv.h"
#include "Z20K11xM_i2c.h"
#include "string.h"
#include "stdbool.h"
#include "Lx07.h"
/*****************************************************************************
 * Global macros
 *****************************************************************************/
#if 1
#define DEV_DES (0x68u)
#define DEV_BKL (0x39u)
#else
#define DEV_DES (0x4Cu)
#define DEV_BKL (0x3A)
#endif

#define DEV_TP  (0x38)
#define DEV_SER 0x40u

#define DEVICE_ID 0x01

#define MAIN_TASK_MS    (5)
#define MAIN_TIME_MS(x) (x / MAIN_TASK_MS)
/*****************************************************************************
 * Global data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
typedef struct I2cSendData
{
    uint8_t  u8DstAddr;
    uint16_t u16DstRegAddr;
    uint8_t  u8RegData;
} I2cSendData;

typedef enum // 根据CAN矩阵定义
{
    CAN_READ_Bkl       = 0x01,
    CAN_READ_LcdTemp   = 0x02,
    CAN_READ_PcbTemp   = 0x03,
    CAN_READ_DesErrReg = 0x04,

    CAN_READ_Batt = 0x06,
    CAN_READ_AdHW = 0x07,

} Config_enCanRead; // 0x730

typedef enum // 根据CAN矩阵定义
{
    CAN_Write_Bkl            = 0x03,
    CAN_Write_LastVpgPicture = 0x04,
    CAN_Write_NextVpgPicture = 0x05,

    CAN_Write_VpgRed   = 0x11,
    CAN_Write_VpgGreen = 0x12,
    CAN_Write_VpgBlue  = 0x13,
    CAN_Write_VpgWhite = 0x14,

    CAN_Write_Fps45HZ = 0x45,
    CAN_Write_Fps60HZ = 0x60,

} Config_enCanWrite; // 0x731
/*****************************************************************************
 * Global function prototypes
 *****************************************************************************/

#endif
/*****************************************************************************
 * End file CONFIG_H
 *****************************************************************************/
