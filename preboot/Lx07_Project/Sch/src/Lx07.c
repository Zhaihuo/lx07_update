/*****************************************************************************
 * @file Lx07.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-09-22
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "Lx07.h"
#include "i2c.h"
#include "Z20K11xM_drv.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"
#include "Z20K11xM_i2c.h"
#include "Uart2.h"
#include "focaltech_core.h"
#include "uart.h"
#include "Debounce.h"
#include "Eeprom.h"
#include "ProductLine.h"
#include "Touch.h"
#include "VedioDisp.h"
#include "Adc.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/

// GPIO6  --> PON         测试点：R64 PON
// GPIO7  --> TP_EXT_RSTN 测试点：R46 TP_EXT_RSTN
// GPIO8  --> RESX        测试点：R65 RESX
// GPIO12 --> display     测试点：D6  MCU_LCD_EN
// GPIO13 --> Backlight   测试点：    BKL_EN

#define LX07_ABS(a, b) ((a > b) ? (a - b) : (b - a))

enum
{
    STEP1 = 2,          // TP_EXT_RSTN up, RESX up  //5ms
    STEP2 = STEP1 + 9,  // PON up   //45ms
    STEP3 = STEP2 + 24, // BKL_EN up    //120ms
};
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
static const I2cSendData Lx07_au8SerVpgFps60HZ[] = {
    {DEV_SER, 0x0053, 0x00}, // 控制数据包的传输
    // {DEV_SER, 0x01E5, 0x02}, // 控制生成的图案

    {DEV_SER, 0x01C8, 0xE3},

#if 1  //  60hz
    {DEV_SER, 0x01CA, 0x00},
    {DEV_SER, 0x01CB, 0x00},
    {DEV_SER, 0x01CC, 0x00},
    {DEV_SER, 0x01CD, 0x00},
    {DEV_SER, 0x01CE, 0x2C},
    {DEV_SER, 0x01CF, 0x00},
    {DEV_SER, 0x01D0, 0x09},
    {DEV_SER, 0x01D1, 0xD7},
    {DEV_SER, 0x01D2, 0x00},
    {DEV_SER, 0x01D3, 0x00},
    {DEV_SER, 0x01D4, 0x00},
    {DEV_SER, 0x01D5, 0x00},
    {DEV_SER, 0x01D6, 0x00},
    {DEV_SER, 0x01D7, 0x20},
    {DEV_SER, 0x01D8, 0x05},
    {DEV_SER, 0x01D9, 0x60},
    {DEV_SER, 0x01DA, 0x01},
    {DEV_SER, 0x01DB, 0xD2},
    {DEV_SER, 0x01DC, 0x00},
    {DEV_SER, 0x01DD, 0xFD},
    {DEV_SER, 0x01DE, 0x60},
    {DEV_SER, 0x01DF, 0x05},
    {DEV_SER, 0x01E0, 0x00},
    {DEV_SER, 0x01E1, 0x00},
    {DEV_SER, 0x01E2, 0x80},
    {DEV_SER, 0x01E3, 0x01},
    {DEV_SER, 0x01E4, 0x9C},
#else  //  45hz
    {DEV_SER, 0x01CA, 0x00},
    {DEV_SER, 0x01CB, 0x00},
    {DEV_SER, 0x01CC, 0x00},
    {DEV_SER, 0x01CD, 0x00},
    {DEV_SER, 0x01CE, 0x2C},
    {DEV_SER, 0x01CF, 0x00},
    {DEV_SER, 0x01D0, 0x0D},
    {DEV_SER, 0x01D1, 0x2B},
    {DEV_SER, 0x01D2, 0x80},
    {DEV_SER, 0x01D3, 0x00},
    {DEV_SER, 0x01D4, 0x00},
    {DEV_SER, 0x01D5, 0x00},
    {DEV_SER, 0x01D6, 0x00},
    {DEV_SER, 0x01D7, 0x20},
    {DEV_SER, 0x01D8, 0x05},
    {DEV_SER, 0x01D9, 0x60},
    {DEV_SER, 0x01DA, 0x02},
    {DEV_SER, 0x01DB, 0x6D},
    {DEV_SER, 0x01DC, 0x04},
    {DEV_SER, 0x01DD, 0x51},
    {DEV_SER, 0x01DE, 0xE0},
    {DEV_SER, 0x01DF, 0x05},
    {DEV_SER, 0x01E0, 0x00},
    {DEV_SER, 0x01E1, 0x00},
    {DEV_SER, 0x01E2, 0x80},
    {DEV_SER, 0x01E3, 0x01},
    {DEV_SER, 0x01E4, 0x9C},
#endif

#ifdef ENABLE_CHECKBOARD
    {DEV_SER, 0x01E5, 0x01},
#else  //  Color bar
    {DEV_SER, 0x01E5, 0x02},
#endif
    {DEV_SER, 0x01E6, 0x04},

#ifdef ENABLE_CHECKBOARD
    // {DEV_SER, 0x01E7, 0x00}, // 颜色A低8位
    // {DEV_SER, 0x01E8, 0x00}, // 颜色A中8位
    // {DEV_SER, 0x01E9, 0x00}, // 颜色A高8位
    // {DEV_SER, 0x01EA, 0xff}, // 颜色B低8位
    // {DEV_SER, 0x01EB, 0xff}, // 颜色B中8位
    // {DEV_SER, 0x01EC, 0xff}, // 颜色B高8位
    // {DEV_SER, 0x01ED, 0x80}, // 颜色A的横向长度
    // {DEV_SER, 0x01EE, 0x80}, // 颜色B的横向长度
    // {DEV_SER, 0x01EF, 0x70}, // 颜色A、B的纵向长度

    {DEV_SER, 0x01E7, 0xff}, // 颜色A低8位  //red
    {DEV_SER, 0x01E8, 0x00}, // 颜色A中8位
    {DEV_SER, 0x01E9, 0x00}, // 颜色A高8位
    {DEV_SER, 0x01EA, 0xff}, // 颜色B低8位
    {DEV_SER, 0x01EB, 0x00}, // 颜色B中8位
    {DEV_SER, 0x01EC, 0x00}, // 颜色B高8位
#endif

    {DEV_SER, 0x01C9, 0x19}, // 设置数据传输时的时钟的触发方式
    {DEV_SER, 0x0001, 0xd8}, // 启用HDMI 启用两个I2C的透传

    /* GPIO08: TP_INT*/
    {DEV_SER, 0x0218, 0x04}, //  RX NO OUT
    {DEV_SER, 0x0219, 0xA8}, //  发送配置的GPIO8
    {DEV_SER, 0x021A, 0x48}, //  接收配置的GPIO8
};

static const I2cSendData Lx07_au8SerVpgFps45HZ[] = {
    {DEV_SER, 0x0053, 0x00}, // 控制数据包的传输
    // {DEV_SER, 0x01E5, 0x02}, // 控制生成的图案

    {DEV_SER, 0x01C8, 0xE3},

#if 0  //  60hz
    {DEV_SER, 0x01CA, 0x00},
    {DEV_SER, 0x01CB, 0x00},
    {DEV_SER, 0x01CC, 0x00},
    {DEV_SER, 0x01CD, 0x00},
    {DEV_SER, 0x01CE, 0x2C},
    {DEV_SER, 0x01CF, 0x00},
    {DEV_SER, 0x01D0, 0x09},
    {DEV_SER, 0x01D1, 0xD7},
    {DEV_SER, 0x01D2, 0x00},
    {DEV_SER, 0x01D3, 0x00},
    {DEV_SER, 0x01D4, 0x00},
    {DEV_SER, 0x01D5, 0x00},
    {DEV_SER, 0x01D6, 0x00},
    {DEV_SER, 0x01D7, 0x20},
    {DEV_SER, 0x01D8, 0x05},
    {DEV_SER, 0x01D9, 0x60},
    {DEV_SER, 0x01DA, 0x01},
    {DEV_SER, 0x01DB, 0xD2},
    {DEV_SER, 0x01DC, 0x00},
    {DEV_SER, 0x01DD, 0xFD},
    {DEV_SER, 0x01DE, 0x60},
    {DEV_SER, 0x01DF, 0x05},
    {DEV_SER, 0x01E0, 0x00},
    {DEV_SER, 0x01E1, 0x00},
    {DEV_SER, 0x01E2, 0x80},
    {DEV_SER, 0x01E3, 0x01},
    {DEV_SER, 0x01E4, 0x9C},
#else  //  45hz
    {DEV_SER, 0x01CA, 0x00},
    {DEV_SER, 0x01CB, 0x00},
    {DEV_SER, 0x01CC, 0x00},
    {DEV_SER, 0x01CD, 0x00},
    {DEV_SER, 0x01CE, 0x2C},
    {DEV_SER, 0x01CF, 0x00},
    {DEV_SER, 0x01D0, 0x0D},
    {DEV_SER, 0x01D1, 0x2B},
    {DEV_SER, 0x01D2, 0x80},
    {DEV_SER, 0x01D3, 0x00},
    {DEV_SER, 0x01D4, 0x00},
    {DEV_SER, 0x01D5, 0x00},
    {DEV_SER, 0x01D6, 0x00},
    {DEV_SER, 0x01D7, 0x20},
    {DEV_SER, 0x01D8, 0x05},
    {DEV_SER, 0x01D9, 0x60},
    {DEV_SER, 0x01DA, 0x02},
    {DEV_SER, 0x01DB, 0x6D},
    {DEV_SER, 0x01DC, 0x04},
    {DEV_SER, 0x01DD, 0x51},
    {DEV_SER, 0x01DE, 0xE0},
    {DEV_SER, 0x01DF, 0x05},
    {DEV_SER, 0x01E0, 0x00},
    {DEV_SER, 0x01E1, 0x00},
    {DEV_SER, 0x01E2, 0x80},
    {DEV_SER, 0x01E3, 0x01},
    {DEV_SER, 0x01E4, 0x9C},
#endif

#ifdef ENABLE_CHECKBOARD
    {DEV_SER, 0x01E5, 0x01},
#else  //  Color bar
    {DEV_SER, 0x01E5, 0x02},
#endif
    {DEV_SER, 0x01E6, 0x04},

#ifdef ENABLE_CHECKBOARD
    // {DEV_SER, 0x01E7, 0x00}, // 颜色A低8位
    // {DEV_SER, 0x01E8, 0x00}, // 颜色A中8位
    // {DEV_SER, 0x01E9, 0x00}, // 颜色A高8位
    // {DEV_SER, 0x01EA, 0xff}, // 颜色B低8位
    // {DEV_SER, 0x01EB, 0xff}, // 颜色B中8位
    // {DEV_SER, 0x01EC, 0xff}, // 颜色B高8位
    // {DEV_SER, 0x01ED, 0x80}, // 颜色A的横向长度
    // {DEV_SER, 0x01EE, 0x80}, // 颜色B的横向长度
    // {DEV_SER, 0x01EF, 0x70}, // 颜色A、B的纵向长度

    {DEV_SER, 0x01E7, 0xff}, // 颜色A低8位  //red
    {DEV_SER, 0x01E8, 0x00}, // 颜色A中8位
    {DEV_SER, 0x01E9, 0x00}, // 颜色A高8位
    {DEV_SER, 0x01EA, 0xff}, // 颜色B低8位
    {DEV_SER, 0x01EB, 0x00}, // 颜色B中8位
    {DEV_SER, 0x01EC, 0x00}, // 颜色B高8位
#endif

    {DEV_SER, 0x01C9, 0x19}, // 设置数据传输时的时钟的触发方式
    {DEV_SER, 0x0001, 0xd8}, // 启用HDMI 启用两个I2C的透传

    /* GPIO08: TP_INT*/
    {DEV_SER, 0x0218, 0x04}, //  RX NO OUT
    {DEV_SER, 0x0219, 0xA8}, //  发送配置的GPIO8
    {DEV_SER, 0x021A, 0x48}, //  接收配置的GPIO8
};

static const I2cSendData Lx07_au8SerHdmi60HZ[] = {
    {DEV_SER, 0x20F5, 0x00},

#if 0  //  45HZ
    {DEV_SER, 0x2E00, 0x00},
    {DEV_SER, 0x2E01, 0xff},
    {DEV_SER, 0x2E02, 0xff},
    {DEV_SER, 0x2E03, 0xff},
    {DEV_SER, 0x2E04, 0xff},
    {DEV_SER, 0x2E05, 0xff},
    {DEV_SER, 0x2E06, 0xff},
    {DEV_SER, 0x2E07, 0x00},
    {DEV_SER, 0x2E08, 0x52},
    {DEV_SER, 0x2E09, 0x55},
    {DEV_SER, 0x2E0A, 0x00},
    {DEV_SER, 0x2E0B, 0x00},
    {DEV_SER, 0x2E0C, 0x01},
    {DEV_SER, 0x2E0D, 0x01},
    {DEV_SER, 0x2E0E, 0x01},
    {DEV_SER, 0x2E0F, 0x01},
    {DEV_SER, 0x2E10, 0x00},
    {DEV_SER, 0x2E11, 0x13},
    {DEV_SER, 0x2E12, 0x01},
    {DEV_SER, 0x2E13, 0x03},
    {DEV_SER, 0x2E14, 0x80},
    {DEV_SER, 0x2E15, 0x12},
    {DEV_SER, 0x2E16, 0x0A},
    {DEV_SER, 0x2E17, 0x78},
    {DEV_SER, 0x2E18, 0x0A},
    {DEV_SER, 0x2E19, 0x31},
    {DEV_SER, 0x2E1A, 0xb9},
    {DEV_SER, 0x2E1B, 0x9c},
    {DEV_SER, 0x2E1C, 0x5e},
    {DEV_SER, 0x2E1D, 0x5b},
    {DEV_SER, 0x2E1E, 0x91},
    {DEV_SER, 0x2E1F, 0x27},
    {DEV_SER, 0x2E20, 0x27},
    {DEV_SER, 0x2E21, 0x4f},
    {DEV_SER, 0x2E22, 0x59},
    {DEV_SER, 0x2E23, 0x00},
    {DEV_SER, 0x2E24, 0x00},
    {DEV_SER, 0x2E25, 0x00},
    {DEV_SER, 0x2E26, 0x01},
    {DEV_SER, 0x2E27, 0x01},
    {DEV_SER, 0x2E28, 0x01},
    {DEV_SER, 0x2E29, 0x01},
    {DEV_SER, 0x2E2A, 0x01},
    {DEV_SER, 0x2E2B, 0x01},
    {DEV_SER, 0x2E2C, 0x01},
    {DEV_SER, 0x2E2D, 0x01},
    {DEV_SER, 0x2E2E, 0x01},
    {DEV_SER, 0x2E2F, 0x01},
    {DEV_SER, 0x2E30, 0x01},
    {DEV_SER, 0x2E31, 0x01},
    {DEV_SER, 0x2E32, 0x01},
    {DEV_SER, 0x2E33, 0x01},
    {DEV_SER, 0x2E34, 0x01},
    {DEV_SER, 0x2E35, 0x01},
    {DEV_SER, 0x2E36, 0x61},
    {DEV_SER, 0x2E37, 0x0f},
    {DEV_SER, 0x2E38, 0x00},
    {DEV_SER, 0x2E39, 0x80},
    {DEV_SER, 0x2E3A, 0x50},
    {DEV_SER, 0x2E3B, 0x9c},
    {DEV_SER, 0x2E3C, 0xd1},
    {DEV_SER, 0x2E3D, 0x10},
    {DEV_SER, 0x2E3E, 0x40},
    {DEV_SER, 0x2E3F, 0x20},
    {DEV_SER, 0x2E40, 0x88},
    {DEV_SER, 0x2E41, 0x00},
    {DEV_SER, 0x2E42, 0x00},
    {DEV_SER, 0x2E43, 0x64},
    {DEV_SER, 0x2E44, 0x50},
    {DEV_SER, 0x2E45, 0x00},
    {DEV_SER, 0x2E46, 0x00},
    {DEV_SER, 0x2E47, 0x18},
    {DEV_SER, 0x2E48, 0x00},
    {DEV_SER, 0x2E49, 0x00},
    {DEV_SER, 0x2E4A, 0x00},
    {DEV_SER, 0x2E4B, 0xfe},
    {DEV_SER, 0x2E4C, 0x00},
    {DEV_SER, 0x2E4D, 0x41},
    {DEV_SER, 0x2E4E, 0x20},
    {DEV_SER, 0x2E4F, 0x74},
    {DEV_SER, 0x2E50, 0x65},
    {DEV_SER, 0x2E51, 0x73},
    {DEV_SER, 0x2E52, 0x74},
    {DEV_SER, 0x2E53, 0x20},
    {DEV_SER, 0x2E54, 0x62},
    {DEV_SER, 0x2E55, 0x79},
    {DEV_SER, 0x2E56, 0x20},
    {DEV_SER, 0x2E57, 0x58},
    {DEV_SER, 0x2E58, 0x44},
    {DEV_SER, 0x2E59, 0x58},
    {DEV_SER, 0x2E5A, 0x00},
    {DEV_SER, 0x2E5B, 0x00},
    {DEV_SER, 0x2E5C, 0x00},
    {DEV_SER, 0x2E5D, 0xfe},
    {DEV_SER, 0x2E5E, 0x00},
    {DEV_SER, 0x2E5F, 0x54},
    {DEV_SER, 0x2E60, 0x46},
    {DEV_SER, 0x2E61, 0x54},
    {DEV_SER, 0x2E62, 0x32},
    {DEV_SER, 0x2E63, 0x70},
    {DEV_SER, 0x2E64, 0x32},
    {DEV_SER, 0x2E65, 0x31},
    {DEV_SER, 0x2E66, 0x31},
    {DEV_SER, 0x2E67, 0x31},
    {DEV_SER, 0x2E68, 0x20},
    {DEV_SER, 0x2E69, 0x0a},
    {DEV_SER, 0x2E6A, 0x20},
    {DEV_SER, 0x2E6B, 0x20},
    {DEV_SER, 0x2E6C, 0x00},
    {DEV_SER, 0x2E6D, 0x00},
    {DEV_SER, 0x2E6E, 0x00},
    {DEV_SER, 0x2E6F, 0xfe},
    {DEV_SER, 0x2E70, 0x00},
    {DEV_SER, 0x2E71, 0x5a},
    {DEV_SER, 0x2E72, 0x65},
    {DEV_SER, 0x2E73, 0x6e},
    {DEV_SER, 0x2E74, 0x67},
    {DEV_SER, 0x2E75, 0x73},
    {DEV_SER, 0x2E76, 0x68},
    {DEV_SER, 0x2E77, 0x75},
    {DEV_SER, 0x2E78, 0x79},
    {DEV_SER, 0x2E79, 0x69},
    {DEV_SER, 0x2E7A, 0x0A},
    {DEV_SER, 0x2E7B, 0x20},
    {DEV_SER, 0x2E7C, 0x20},
    {DEV_SER, 0x2E7D, 0x20},
    {DEV_SER, 0x2E7E, 0x00},
    {DEV_SER, 0x2E7F, 0x0c},
#else  //  60HZ
    {DEV_SER, 0x2E00, 0x00},
    {DEV_SER, 0x2E01, 0xff},
    {DEV_SER, 0x2E02, 0xff},
    {DEV_SER, 0x2E03, 0xff},
    {DEV_SER, 0x2E04, 0xff},
    {DEV_SER, 0x2E05, 0xff},
    {DEV_SER, 0x2E06, 0xff},
    {DEV_SER, 0x2E07, 0x00},
    {DEV_SER, 0x2E08, 0x52},
    {DEV_SER, 0x2E09, 0x55},
    {DEV_SER, 0x2E0A, 0x00},
    {DEV_SER, 0x2E0B, 0x00},
    {DEV_SER, 0x2E0C, 0x01},
    {DEV_SER, 0x2E0D, 0x01},
    {DEV_SER, 0x2E0E, 0x01},
    {DEV_SER, 0x2E0F, 0x01},
    {DEV_SER, 0x2E10, 0x00},
    {DEV_SER, 0x2E11, 0x13},
    {DEV_SER, 0x2E12, 0x01},
    {DEV_SER, 0x2E13, 0x03},
    {DEV_SER, 0x2E14, 0x80},
    {DEV_SER, 0x2E15, 0x12},
    {DEV_SER, 0x2E16, 0x0A},
    {DEV_SER, 0x2E17, 0x78},
    {DEV_SER, 0x2E18, 0x0A},
    {DEV_SER, 0x2E19, 0x31},
    {DEV_SER, 0x2E1A, 0xb9},
    {DEV_SER, 0x2E1B, 0x9c},
    {DEV_SER, 0x2E1C, 0x5e},
    {DEV_SER, 0x2E1D, 0x5b},
    {DEV_SER, 0x2E1E, 0x91},
    {DEV_SER, 0x2E1F, 0x27},
    {DEV_SER, 0x2E20, 0x27},
    {DEV_SER, 0x2E21, 0x4f},
    {DEV_SER, 0x2E22, 0x59},
    {DEV_SER, 0x2E23, 0x00},
    {DEV_SER, 0x2E24, 0x00},
    {DEV_SER, 0x2E25, 0x00},
    {DEV_SER, 0x2E26, 0x01},
    {DEV_SER, 0x2E27, 0x01},
    {DEV_SER, 0x2E28, 0x01},
    {DEV_SER, 0x2E29, 0x01},
    {DEV_SER, 0x2E2A, 0x01},
    {DEV_SER, 0x2E2B, 0x01},
    {DEV_SER, 0x2E2C, 0x01},
    {DEV_SER, 0x2E2D, 0x01},
    {DEV_SER, 0x2E2E, 0x01},
    {DEV_SER, 0x2E2F, 0x01},
    {DEV_SER, 0x2E30, 0x01},
    {DEV_SER, 0x2E31, 0x01},
    {DEV_SER, 0x2E32, 0x01},
    {DEV_SER, 0x2E33, 0x01},
    {DEV_SER, 0x2E34, 0x01},
    {DEV_SER, 0x2E35, 0x01},
    {DEV_SER, 0x2E36, 0x61},
    {DEV_SER, 0x2E37, 0x0f},
    {DEV_SER, 0x2E38, 0x00},
    {DEV_SER, 0x2E39, 0x80},
    {DEV_SER, 0x2E3A, 0x50},
    {DEV_SER, 0x2E3B, 0x9c},
    {DEV_SER, 0x2E3C, 0x36},
    {DEV_SER, 0x2E3D, 0x10},
    {DEV_SER, 0x2E3E, 0x40},
    {DEV_SER, 0x2E3F, 0x20},
    {DEV_SER, 0x2E40, 0x68},
    {DEV_SER, 0x2E41, 0x08},
    {DEV_SER, 0x2E42, 0x00},
    {DEV_SER, 0x2E43, 0x9c},
    {DEV_SER, 0x2E44, 0x51},
    {DEV_SER, 0x2E45, 0x00},
    {DEV_SER, 0x2E46, 0x00},
    {DEV_SER, 0x2E47, 0x18},
    {DEV_SER, 0x2E48, 0x00},
    {DEV_SER, 0x2E49, 0x00},
    {DEV_SER, 0x2E4A, 0x00},
    {DEV_SER, 0x2E4B, 0xfe},
    {DEV_SER, 0x2E4C, 0x00},
    {DEV_SER, 0x2E4D, 0x41},
    {DEV_SER, 0x2E4E, 0x20},
    {DEV_SER, 0x2E4F, 0x74},
    {DEV_SER, 0x2E50, 0x65},
    {DEV_SER, 0x2E51, 0x73},
    {DEV_SER, 0x2E52, 0x74},
    {DEV_SER, 0x2E53, 0x20},
    {DEV_SER, 0x2E54, 0x62},
    {DEV_SER, 0x2E55, 0x79},
    {DEV_SER, 0x2E56, 0x20},
    {DEV_SER, 0x2E57, 0x58},
    {DEV_SER, 0x2E58, 0x44},
    {DEV_SER, 0x2E59, 0x58},
    {DEV_SER, 0x2E5A, 0x00},
    {DEV_SER, 0x2E5B, 0x00},
    {DEV_SER, 0x2E5C, 0x00},
    {DEV_SER, 0x2E5D, 0xfe},
    {DEV_SER, 0x2E5E, 0x00},
    {DEV_SER, 0x2E5F, 0x54},
    {DEV_SER, 0x2E60, 0x46},
    {DEV_SER, 0x2E61, 0x54},
    {DEV_SER, 0x2E62, 0x32},
    {DEV_SER, 0x2E63, 0x70},
    {DEV_SER, 0x2E64, 0x32},
    {DEV_SER, 0x2E65, 0x31},
    {DEV_SER, 0x2E66, 0x31},
    {DEV_SER, 0x2E67, 0x31},
    {DEV_SER, 0x2E68, 0x20},
    {DEV_SER, 0x2E69, 0x0a},
    {DEV_SER, 0x2E6A, 0x20},
    {DEV_SER, 0x2E6B, 0x20},
    {DEV_SER, 0x2E6C, 0x00},
    {DEV_SER, 0x2E6D, 0x00},
    {DEV_SER, 0x2E6E, 0x00},
    {DEV_SER, 0x2E6F, 0xfe},
    {DEV_SER, 0x2E70, 0x00},
    {DEV_SER, 0x2E71, 0x5a},
    {DEV_SER, 0x2E72, 0x65},
    {DEV_SER, 0x2E73, 0x6e},
    {DEV_SER, 0x2E74, 0x67},
    {DEV_SER, 0x2E75, 0x73},
    {DEV_SER, 0x2E76, 0x68},
    {DEV_SER, 0x2E77, 0x75},
    {DEV_SER, 0x2E78, 0x79},
    {DEV_SER, 0x2E79, 0x69},
    {DEV_SER, 0x2E7A, 0x0A},
    {DEV_SER, 0x2E7B, 0x20},
    {DEV_SER, 0x2E7C, 0x20},
    {DEV_SER, 0x2E7D, 0x20},
    {DEV_SER, 0x2E7E, 0x00},
    {DEV_SER, 0x2E7F, 0x40},
#endif

    {DEV_SER, 0x20F5, 0x01},
    // {DEV_SER,0x0001,0xF8},
    {DEV_SER, 0x0001, 0xD8},

    /* GPIO08: TP_INT*/  /*串行器的GPIO08配置成输入引脚  接收的ID是0x08*/
    {DEV_SER, 0x0218, 0x04}, //  RX NO OUT
    {DEV_SER, 0x0219, 0xA8}, //  发送配置的GPIO8
    {DEV_SER, 0x021A, 0x48}, //  接收配置的GPIO8
};

static const I2cSendData Lx07_au8SerHdmi45HZ[] = {
    {DEV_SER, 0x20F5, 0x00},

#if 1  //  45HZ
    {DEV_SER, 0x2E00, 0x00},
    {DEV_SER, 0x2E01, 0xff},
    {DEV_SER, 0x2E02, 0xff},
    {DEV_SER, 0x2E03, 0xff},
    {DEV_SER, 0x2E04, 0xff},
    {DEV_SER, 0x2E05, 0xff},
    {DEV_SER, 0x2E06, 0xff},
    {DEV_SER, 0x2E07, 0x00},
    {DEV_SER, 0x2E08, 0x52},
    {DEV_SER, 0x2E09, 0x55},
    {DEV_SER, 0x2E0A, 0x00},
    {DEV_SER, 0x2E0B, 0x00},
    {DEV_SER, 0x2E0C, 0x01},
    {DEV_SER, 0x2E0D, 0x01},
    {DEV_SER, 0x2E0E, 0x01},
    {DEV_SER, 0x2E0F, 0x01},
    {DEV_SER, 0x2E10, 0x00},
    {DEV_SER, 0x2E11, 0x13},
    {DEV_SER, 0x2E12, 0x01},
    {DEV_SER, 0x2E13, 0x03},
    {DEV_SER, 0x2E14, 0x80},
    {DEV_SER, 0x2E15, 0x12},
    {DEV_SER, 0x2E16, 0x0A},
    {DEV_SER, 0x2E17, 0x78},
    {DEV_SER, 0x2E18, 0x0A},
    {DEV_SER, 0x2E19, 0x31},
    {DEV_SER, 0x2E1A, 0xb9},
    {DEV_SER, 0x2E1B, 0x9c},
    {DEV_SER, 0x2E1C, 0x5e},
    {DEV_SER, 0x2E1D, 0x5b},
    {DEV_SER, 0x2E1E, 0x91},
    {DEV_SER, 0x2E1F, 0x27},
    {DEV_SER, 0x2E20, 0x27},
    {DEV_SER, 0x2E21, 0x4f},
    {DEV_SER, 0x2E22, 0x59},
    {DEV_SER, 0x2E23, 0x00},
    {DEV_SER, 0x2E24, 0x00},
    {DEV_SER, 0x2E25, 0x00},
    {DEV_SER, 0x2E26, 0x01},
    {DEV_SER, 0x2E27, 0x01},
    {DEV_SER, 0x2E28, 0x01},
    {DEV_SER, 0x2E29, 0x01},
    {DEV_SER, 0x2E2A, 0x01},
    {DEV_SER, 0x2E2B, 0x01},
    {DEV_SER, 0x2E2C, 0x01},
    {DEV_SER, 0x2E2D, 0x01},
    {DEV_SER, 0x2E2E, 0x01},
    {DEV_SER, 0x2E2F, 0x01},
    {DEV_SER, 0x2E30, 0x01},
    {DEV_SER, 0x2E31, 0x01},
    {DEV_SER, 0x2E32, 0x01},
    {DEV_SER, 0x2E33, 0x01},
    {DEV_SER, 0x2E34, 0x01},
    {DEV_SER, 0x2E35, 0x01},
    {DEV_SER, 0x2E36, 0x61},
    {DEV_SER, 0x2E37, 0x0f},
    {DEV_SER, 0x2E38, 0x00},
    {DEV_SER, 0x2E39, 0x80},
    {DEV_SER, 0x2E3A, 0x50},
    {DEV_SER, 0x2E3B, 0x9c},
    {DEV_SER, 0x2E3C, 0xd1},
    {DEV_SER, 0x2E3D, 0x10},
    {DEV_SER, 0x2E3E, 0x40},
    {DEV_SER, 0x2E3F, 0x20},
    {DEV_SER, 0x2E40, 0xf8}, //
    {DEV_SER, 0x2E41, 0x0c}, //
    {DEV_SER, 0x2E42, 0x00},
    {DEV_SER, 0x2E43, 0x9c},
    {DEV_SER, 0x2E44, 0x51},
    {DEV_SER, 0x2E45, 0x00},
    {DEV_SER, 0x2E46, 0x00},
    {DEV_SER, 0x2E47, 0x18},
    {DEV_SER, 0x2E48, 0x00},
    {DEV_SER, 0x2E49, 0x00},
    {DEV_SER, 0x2E4A, 0x00},
    {DEV_SER, 0x2E4B, 0xfe},
    {DEV_SER, 0x2E4C, 0x00},
    {DEV_SER, 0x2E4D, 0x41},
    {DEV_SER, 0x2E4E, 0x20},
    {DEV_SER, 0x2E4F, 0x74},
    {DEV_SER, 0x2E50, 0x65},
    {DEV_SER, 0x2E51, 0x73},
    {DEV_SER, 0x2E52, 0x74},
    {DEV_SER, 0x2E53, 0x20},
    {DEV_SER, 0x2E54, 0x62},
    {DEV_SER, 0x2E55, 0x79},
    {DEV_SER, 0x2E56, 0x20},
    {DEV_SER, 0x2E57, 0x58},
    {DEV_SER, 0x2E58, 0x44},
    {DEV_SER, 0x2E59, 0x58},
    {DEV_SER, 0x2E5A, 0x00},
    {DEV_SER, 0x2E5B, 0x00},
    {DEV_SER, 0x2E5C, 0x00},
    {DEV_SER, 0x2E5D, 0xfe},
    {DEV_SER, 0x2E5E, 0x00},
    {DEV_SER, 0x2E5F, 0x54},
    {DEV_SER, 0x2E60, 0x46},
    {DEV_SER, 0x2E61, 0x54},
    {DEV_SER, 0x2E62, 0x32},
    {DEV_SER, 0x2E63, 0x70},
    {DEV_SER, 0x2E64, 0x32},
    {DEV_SER, 0x2E65, 0x31},
    {DEV_SER, 0x2E66, 0x31},
    {DEV_SER, 0x2E67, 0x31},
    {DEV_SER, 0x2E68, 0x20},
    {DEV_SER, 0x2E69, 0x0a},
    {DEV_SER, 0x2E6A, 0x20},
    {DEV_SER, 0x2E6B, 0x20},
    {DEV_SER, 0x2E6C, 0x00},
    {DEV_SER, 0x2E6D, 0x00},
    {DEV_SER, 0x2E6E, 0x00},
    {DEV_SER, 0x2E6F, 0xfe},
    {DEV_SER, 0x2E70, 0x00},
    {DEV_SER, 0x2E71, 0x5a},
    {DEV_SER, 0x2E72, 0x65},
    {DEV_SER, 0x2E73, 0x6e},
    {DEV_SER, 0x2E74, 0x67},
    {DEV_SER, 0x2E75, 0x73},
    {DEV_SER, 0x2E76, 0x68},
    {DEV_SER, 0x2E77, 0x75},
    {DEV_SER, 0x2E78, 0x79},
    {DEV_SER, 0x2E79, 0x69},
    {DEV_SER, 0x2E7A, 0x0A},
    {DEV_SER, 0x2E7B, 0x20},
    {DEV_SER, 0x2E7C, 0x20},
    {DEV_SER, 0x2E7D, 0x20},
    {DEV_SER, 0x2E7E, 0x00},
    {DEV_SER, 0x2E7F, 0x8d},
#else  //  60HZ
    {DEV_SER, 0x2E00, 0x00},
    {DEV_SER, 0x2E01, 0xff},
    {DEV_SER, 0x2E02, 0xff},
    {DEV_SER, 0x2E03, 0xff},
    {DEV_SER, 0x2E04, 0xff},
    {DEV_SER, 0x2E05, 0xff},
    {DEV_SER, 0x2E06, 0xff},
    {DEV_SER, 0x2E07, 0x00},
    {DEV_SER, 0x2E08, 0x52},
    {DEV_SER, 0x2E09, 0x55},
    {DEV_SER, 0x2E0A, 0x00},
    {DEV_SER, 0x2E0B, 0x00},
    {DEV_SER, 0x2E0C, 0x01},
    {DEV_SER, 0x2E0D, 0x01},
    {DEV_SER, 0x2E0E, 0x01},
    {DEV_SER, 0x2E0F, 0x01},
    {DEV_SER, 0x2E10, 0x00},
    {DEV_SER, 0x2E11, 0x13},
    {DEV_SER, 0x2E12, 0x01},
    {DEV_SER, 0x2E13, 0x03},
    {DEV_SER, 0x2E14, 0x80},
    {DEV_SER, 0x2E15, 0x12},
    {DEV_SER, 0x2E16, 0x0A},
    {DEV_SER, 0x2E17, 0x78},
    {DEV_SER, 0x2E18, 0x0A},
    {DEV_SER, 0x2E19, 0x31},
    {DEV_SER, 0x2E1A, 0xb9},
    {DEV_SER, 0x2E1B, 0x9c},
    {DEV_SER, 0x2E1C, 0x5e},
    {DEV_SER, 0x2E1D, 0x5b},
    {DEV_SER, 0x2E1E, 0x91},
    {DEV_SER, 0x2E1F, 0x27},
    {DEV_SER, 0x2E20, 0x27},
    {DEV_SER, 0x2E21, 0x4f},
    {DEV_SER, 0x2E22, 0x59},
    {DEV_SER, 0x2E23, 0x00},
    {DEV_SER, 0x2E24, 0x00},
    {DEV_SER, 0x2E25, 0x00},
    {DEV_SER, 0x2E26, 0x01},
    {DEV_SER, 0x2E27, 0x01},
    {DEV_SER, 0x2E28, 0x01},
    {DEV_SER, 0x2E29, 0x01},
    {DEV_SER, 0x2E2A, 0x01},
    {DEV_SER, 0x2E2B, 0x01},
    {DEV_SER, 0x2E2C, 0x01},
    {DEV_SER, 0x2E2D, 0x01},
    {DEV_SER, 0x2E2E, 0x01},
    {DEV_SER, 0x2E2F, 0x01},
    {DEV_SER, 0x2E30, 0x01},
    {DEV_SER, 0x2E31, 0x01},
    {DEV_SER, 0x2E32, 0x01},
    {DEV_SER, 0x2E33, 0x01},
    {DEV_SER, 0x2E34, 0x01},
    {DEV_SER, 0x2E35, 0x01},
    {DEV_SER, 0x2E36, 0x61},
    {DEV_SER, 0x2E37, 0x0f},
    {DEV_SER, 0x2E38, 0x00},
    {DEV_SER, 0x2E39, 0x80},
    {DEV_SER, 0x2E3A, 0x50},
    {DEV_SER, 0x2E3B, 0x9c},
    {DEV_SER, 0x2E3C, 0x36},
    {DEV_SER, 0x2E3D, 0x10},
    {DEV_SER, 0x2E3E, 0x40},
    {DEV_SER, 0x2E3F, 0x20},
    {DEV_SER, 0x2E40, 0x68},
    {DEV_SER, 0x2E41, 0x08},
    {DEV_SER, 0x2E42, 0x00},
    {DEV_SER, 0x2E43, 0x9c},
    {DEV_SER, 0x2E44, 0x51},
    {DEV_SER, 0x2E45, 0x00},
    {DEV_SER, 0x2E46, 0x00},
    {DEV_SER, 0x2E47, 0x18},
    {DEV_SER, 0x2E48, 0x00},
    {DEV_SER, 0x2E49, 0x00},
    {DEV_SER, 0x2E4A, 0x00},
    {DEV_SER, 0x2E4B, 0xfe},
    {DEV_SER, 0x2E4C, 0x00},
    {DEV_SER, 0x2E4D, 0x41},
    {DEV_SER, 0x2E4E, 0x20},
    {DEV_SER, 0x2E4F, 0x74},
    {DEV_SER, 0x2E50, 0x65},
    {DEV_SER, 0x2E51, 0x73},
    {DEV_SER, 0x2E52, 0x74},
    {DEV_SER, 0x2E53, 0x20},
    {DEV_SER, 0x2E54, 0x62},
    {DEV_SER, 0x2E55, 0x79},
    {DEV_SER, 0x2E56, 0x20},
    {DEV_SER, 0x2E57, 0x58},
    {DEV_SER, 0x2E58, 0x44},
    {DEV_SER, 0x2E59, 0x58},
    {DEV_SER, 0x2E5A, 0x00},
    {DEV_SER, 0x2E5B, 0x00},
    {DEV_SER, 0x2E5C, 0x00},
    {DEV_SER, 0x2E5D, 0xfe},
    {DEV_SER, 0x2E5E, 0x00},
    {DEV_SER, 0x2E5F, 0x54},
    {DEV_SER, 0x2E60, 0x46},
    {DEV_SER, 0x2E61, 0x54},
    {DEV_SER, 0x2E62, 0x32},
    {DEV_SER, 0x2E63, 0x70},
    {DEV_SER, 0x2E64, 0x32},
    {DEV_SER, 0x2E65, 0x31},
    {DEV_SER, 0x2E66, 0x31},
    {DEV_SER, 0x2E67, 0x31},
    {DEV_SER, 0x2E68, 0x20},
    {DEV_SER, 0x2E69, 0x0a},
    {DEV_SER, 0x2E6A, 0x20},
    {DEV_SER, 0x2E6B, 0x20},
    {DEV_SER, 0x2E6C, 0x00},
    {DEV_SER, 0x2E6D, 0x00},
    {DEV_SER, 0x2E6E, 0x00},
    {DEV_SER, 0x2E6F, 0xfe},
    {DEV_SER, 0x2E70, 0x00},
    {DEV_SER, 0x2E71, 0x5a},
    {DEV_SER, 0x2E72, 0x65},
    {DEV_SER, 0x2E73, 0x6e},
    {DEV_SER, 0x2E74, 0x67},
    {DEV_SER, 0x2E75, 0x73},
    {DEV_SER, 0x2E76, 0x68},
    {DEV_SER, 0x2E77, 0x75},
    {DEV_SER, 0x2E78, 0x79},
    {DEV_SER, 0x2E79, 0x69},
    {DEV_SER, 0x2E7A, 0x0A},
    {DEV_SER, 0x2E7B, 0x20},
    {DEV_SER, 0x2E7C, 0x20},
    {DEV_SER, 0x2E7D, 0x20},
    {DEV_SER, 0x2E7E, 0x00},
    {DEV_SER, 0x2E7F, 0x40},
#endif

    {DEV_SER, 0x20F5, 0x01},
    // {DEV_SER,0x0001,0xF8},
    {DEV_SER, 0x0001, 0xD8},

    /* GPIO08: TP_INT*/  /*串行器的GPIO08配置成输入引脚  接收的ID是0x08*/
    {DEV_SER, 0x0218, 0x04}, //  RX NO OUT
    {DEV_SER, 0x0219, 0xA8}, //  发送配置的GPIO8
    {DEV_SER, 0x021A, 0x48}, //  接收配置的GPIO8
};

static const I2cSendData Lx07_au8DesSeq[] =
    {
        {DEV_DES, 0x0001, 0x12},
        {DEV_DES, 0x0050, 0x00},
        {DEV_DES, 0x01CE, 0x47},
        {DEV_DES, 0x0D03, 0x8B}, //  展频 0x89:0.25%    0x8A:0.5%   0x8B:1%     0x8C:2%   0x8D:4%
        {DEV_DES, 0x01CF, 0x09},

        {DEV_DES, 0x01D2, 0x04}, //  LANE_SEL_A2  Output form ACLK  0x24(2是default，配置LANE_SEL_B2  Output from B2)
        {DEV_DES, 0x01D4, 0x02}, //  LANE_SEL_ACLK Output from A2   0x42(4是default，配置B通道的时钟)

        /* GPIO04: TP_INT */
        // {DEV_DES, 0x0212, 0x10}, //  PON拉高     /*解串器的GPIO04配置成输出引脚  发送的ID是0x08*/
        {DEV_DES, 0x020C, 0x03}, //  TX OUT
        {DEV_DES, 0x020D, 0xA8}, //  发送配置的GPIO8
        {DEV_DES, 0x020E, 0x48}, //  接收配置的GPIO8

        /* GPIO9: BKL_FAULT */
        {DEV_DES, 0x021B, 0x05}, // 禁用输出 输入模式

        {DEV_DES, 0x0203, 0x00}, //  0x00:Low, Enable EEp Write     0x10:High, Disable EEp Write
};

static const Lx07_stI2cWrite Lx07_au8BklCfg[] =
    {
        {DEV_BKL, 0x00, 0x87}, //  0xAF是PWM调光        0x87是三路LED   0x83是两路LED
        {DEV_BKL, 0x05, 0x26}, //  IN_I2CDIM_H    0x1FFF/0x3FFF = 50%     0x2666/0x3FFF = 60%
        {DEV_BKL, 0x06, 0x66}, //  IN_I2CDIM_L

        // {DEV_BKL, 0x07, 0x01},
        // {DEV_BKL, 0x08, 0xFF},
        // {DEV_BKL, 0x01, 0x74},
};

static bool Lx07_InitAllEndFlg = false;
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static bool Lx07_boInitLcd(void);
static bool Lx07_boVpgCfg(void);
static bool Lx07_boSerHdmiCfg(void);
static bool Lx07_boDesSetSequence(void); // 由于硬件PCB-v2.0.0的TxCLK_OUTA-连接的是LVDS_D2_N，TxOUT_A2-连接的是LVDS_CLK_N，因此需要重新配置连接顺序
static bool Lx07_vBklCfg(void);

static bool Lx07_boWriteEepFlg = true;

static bool Lx07_boInitProFlg = false; // 初始化后，已经开始读写寄存器
/*****************************************************************************
 * function definitions
 *****************************************************************************/
void Lx07_vInit(void)
{
    static bool boInitFlg = true;

    if (boInitFlg)
    {
        boInitFlg = false;
        ProductLine_vInit();
    }
    else
    {
        /*Nothing*/
    }
}

void Lx07_vWatchDog(void)
{
    static uint8_t u8Count = 0u;

    u8Count += 1u;
    if (u8Count >= 100u)
    {
        u8Count = 0u;
        WDOG_Refresh();
    }
    else
    {}
}

void Lx07_vTask5ms(void)
{
    if ((GPIO_ReadPinLevel(PORT_C, GPIO_5) == GPIO_LOW) && (Lx07_boInitProFlg)) // 总成断电会复位一次
    {
        UART_PRINTF("Poweroff Reset\r\n");
        NVIC_SystemReset();
    }

    Lx07_vWatchDog();

    if (Lx07_boInitLcd())
    {
        if (!Lx07_InitAllEndFlg)
        {
            ProductLine_vHandle();

            if (VedioDisp_boReadFpsEnd())
            {
#ifdef ENABLE_HDMI
                if (Lx07_boSerHdmiCfg())
                {
                    if (Lx07_boDesSetSequence())
                    {
                        Lx07_vBklCfg();
                    }
                }
#else
                if (Lx07_boVpgCfg())
                {
                    if (Lx07_boDesSetSequence())
                    {
                        Lx07_vBklCfg();
                    }
                }
#endif
            }
        }
    }

    if (Lx07_InitAllEndFlg)
    {
        Lx07_vInit();
        ProductLine_vHandle();
    }
}

static bool Lx07_boInitLcd(void)
{
    static uint16_t u16Index       = 0;
    static uint8_t  u8TimeCnt      = 0;
    static uint16_t u16TimeWait5S  = MAIN_TIME_MS(200);
    static uint16_t u16LockLowTime = 0;

    uint16_t u16Addr = 0;
    uint8_t  u8Data  = 0;

    if (u8TimeCnt == 0)
    {
        if (GPIO_ReadPinLevel(PORT_C, GPIO_5) == GPIO_HIGH && u16TimeWait5S)
        {
            if (u16LockLowTime >= MAIN_TIME_MS(1000)) // 确保换件时，对手件还会复位一次
            {
                UART_PRINTF("Poweron Reset\r\n");
                NVIC_SystemReset();
            }

            u16TimeWait5S--;
            if (!u16TimeWait5S)
            {
                u8TimeCnt++;
                Lx07_boInitProFlg = true;
                I2C_Disable(I2C0_ID);
                I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                I2C_Enable(I2C0_ID);
            }
        }

        if (GPIO_ReadPinLevel(PORT_C, GPIO_5) == GPIO_LOW)
        {
            u16TimeWait5S  = MAIN_TIME_MS(200);
            u16LockLowTime = (++u16LockLowTime >= MAIN_TIME_MS(1000)) ? MAIN_TIME_MS(1000) : u16LockLowTime;
            // UART_PRINTF("u16LockLowTime = %d\r\n", u16LockLowTime);
        }
    }
    else if (u8TimeCnt < 3)
    {
        u8TimeCnt++;
    }
    else if (u8TimeCnt == 3)
    {
        u16Addr = 0x140;
        u8Data  = 0x20;
        Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&u16Addr, (uint8_t *)&u8Data);
        u16Addr = 0x2;
        u8Data  = 0x43;
        Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&u16Addr, (uint8_t *)&u8Data);
        u8TimeCnt++;
    }
    else if (u8TimeCnt < 6)
    {
        u8TimeCnt++;
    }
    else if (u8TimeCnt == 6)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, DEV_DES);
        I2C_Enable(I2C0_ID);
        u8TimeCnt++;
    }
    else if (u8TimeCnt < 9)
    {
        u8TimeCnt++;
    }
    else if (u8TimeCnt == 9)
    {
        switch (u16Index)
        {
            case 0:
                DES_GPIO_LOW(12); // LCD_PR_EN down

                DES_GPIO_LOW(8);  // RESX  down
                DES_GPIO_LOW(6);  // PON  down
                DES_GPIO_LOW(13); // BKL_EN down
                DES_GPIO_LOW(7);  // TP_EXT_RSTN down
                break;
            case STEP1:
                DES_GPIO_HIGH(7); // TP_EXT_RSTN up
                DES_GPIO_HIGH(8); // RESX up
                break;
            case STEP2:
                DES_GPIO_HIGH(6); // PON up
                break;
            case STEP3:
                DES_GPIO_HIGH(13); // BKL_EN up
                break;
        }
        u16Index > (STEP3 + 1) ? u16Index = u16Index : u16Index++; // 改成STEP3 + 1的原因：当对手件上电大概20分钟后接上总成，无背光，所以增加拉背光后的等待时间
    }

    return u16Index > (STEP3 + 1) ? true : false;
}

static bool Lx07_boVpgCfg(void)
{
    static uint8_t u8VpgTimeCnt = 0;
    static uint8_t u8VpgCfgCnt  = 0;

    static bool boRte = false;

    if (!boRte)
    {
        if (u8VpgTimeCnt < 2)
        {
            u8VpgTimeCnt++;
        }
        else if (u8VpgTimeCnt == 2)
        {
            u8VpgTimeCnt = 3;
            I2C_Disable(I2C0_ID);
            I2C_SetTargetAddr(I2C0_ID, DEV_SER);
            I2C_Enable(I2C0_ID);
        }
        else if (u8VpgTimeCnt < 4)
        {
            u8VpgTimeCnt++;
        }
        else if (u8VpgTimeCnt == 4)
        {
            u8VpgTimeCnt = 4;

            if (CAN_Write_Fps60HZ == VedioDisp_u8FpsHz)
            {
                if (u8VpgCfgCnt < (sizeof(Lx07_au8SerVpgFps60HZ) / sizeof(Lx07_au8SerVpgFps60HZ[0])))
                {
                    Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&Lx07_au8SerVpgFps60HZ[u8VpgCfgCnt].u16DstRegAddr, (uint8_t *)&Lx07_au8SerVpgFps60HZ[u8VpgCfgCnt].u8RegData);
                    u8VpgCfgCnt++;
                }
                else if (u8VpgCfgCnt == (sizeof(Lx07_au8SerVpgFps60HZ) / sizeof(Lx07_au8SerVpgFps60HZ[0])))
                {
                    u8VpgCfgCnt = (sizeof(Lx07_au8SerVpgFps60HZ) / sizeof(Lx07_au8SerVpgFps60HZ[0])) + 1;
                    I2C_Disable(I2C0_ID);
                    I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                    I2C_Enable(I2C0_ID);
                    boRte = true;
                }
                else
                {}
            }
            else if (CAN_Write_Fps45HZ == VedioDisp_u8FpsHz)
            {
                if (u8VpgCfgCnt < (sizeof(Lx07_au8SerVpgFps45HZ) / sizeof(Lx07_au8SerVpgFps45HZ[0])))
                {
                    Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&Lx07_au8SerVpgFps45HZ[u8VpgCfgCnt].u16DstRegAddr, (uint8_t *)&Lx07_au8SerVpgFps45HZ[u8VpgCfgCnt].u8RegData);
                    u8VpgCfgCnt++;
                }
                else if (u8VpgCfgCnt == (sizeof(Lx07_au8SerVpgFps45HZ) / sizeof(Lx07_au8SerVpgFps45HZ[0])))
                {
                    u8VpgCfgCnt = (sizeof(Lx07_au8SerVpgFps45HZ) / sizeof(Lx07_au8SerVpgFps45HZ[0])) + 1;
                    I2C_Disable(I2C0_ID);
                    I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                    I2C_Enable(I2C0_ID);
                    boRte = true;
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return boRte;
}

static bool Lx07_boSerHdmiCfg(void)
{
    static uint8_t u8HdmiTimeCnt = 0;
    static uint8_t u8HdmiCfgCnt  = 0;

    static bool boRte = false;

    if (!boRte)
    {
        if (u8HdmiTimeCnt < 2)
        {
            u8HdmiTimeCnt++;
        }
        else if (u8HdmiTimeCnt == 2)
        {
            u8HdmiTimeCnt = 3;
            I2C_Disable(I2C0_ID);
            I2C_SetTargetAddr(I2C0_ID, DEV_SER);
            I2C_Enable(I2C0_ID);
        }
        else if (u8HdmiTimeCnt < 4)
        {
            u8HdmiTimeCnt++;
        }
        else if (u8HdmiTimeCnt == 4)
        {
            u8HdmiTimeCnt = 4;

            if (CAN_Write_Fps60HZ == VedioDisp_u8FpsHz)
            {
                if (u8HdmiCfgCnt < (sizeof(Lx07_au8SerHdmi60HZ) / sizeof(Lx07_au8SerHdmi60HZ[0])))
                {
                    Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&Lx07_au8SerHdmi60HZ[u8HdmiCfgCnt].u16DstRegAddr, (uint8_t *)&Lx07_au8SerHdmi60HZ[u8HdmiCfgCnt].u8RegData);
                    u8HdmiCfgCnt++;
                }
                else if (u8HdmiCfgCnt == (sizeof(Lx07_au8SerHdmi60HZ) / sizeof(Lx07_au8SerHdmi60HZ[0])))
                {
                    u8HdmiCfgCnt = (sizeof(Lx07_au8SerHdmi60HZ) / sizeof(Lx07_au8SerHdmi60HZ[0])) + 1;
                    I2C_Disable(I2C0_ID);
                    I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                    I2C_Enable(I2C0_ID);
                    boRte = true;
                }
                else
                {}
            }
            else if (CAN_Write_Fps45HZ == VedioDisp_u8FpsHz)
            {
                if (u8HdmiCfgCnt < (sizeof(Lx07_au8SerHdmi45HZ) / sizeof(Lx07_au8SerHdmi45HZ[0])))
                {
                    Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&Lx07_au8SerHdmi45HZ[u8HdmiCfgCnt].u16DstRegAddr, (uint8_t *)&Lx07_au8SerHdmi45HZ[u8HdmiCfgCnt].u8RegData);
                    u8HdmiCfgCnt++;
                }
                else if (u8HdmiCfgCnt == (sizeof(Lx07_au8SerHdmi45HZ) / sizeof(Lx07_au8SerHdmi45HZ[0])))
                {
                    u8HdmiCfgCnt = (sizeof(Lx07_au8SerHdmi45HZ) / sizeof(Lx07_au8SerHdmi45HZ[0])) + 1;
                    I2C_Disable(I2C0_ID);
                    I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                    I2C_Enable(I2C0_ID);
                    boRte = true;
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return boRte;
}

static bool Lx07_boDesSetSequence(void)
{
    static uint8_t u8DesSeqTimeCnt = 0;
    static uint8_t u8DesSeqCfgCnt  = 0;

    static bool boRte = false;

    if (!boRte)
    {
        if (u8DesSeqTimeCnt < 2)
        {
            u8DesSeqTimeCnt++;
        }
        else if (u8DesSeqTimeCnt == 2)
        {
            u8DesSeqTimeCnt = 3;
            I2C_Disable(I2C0_ID);
            I2C_SetTargetAddr(I2C0_ID, DEV_DES);
            I2C_Enable(I2C0_ID);
        }
        else if (u8DesSeqTimeCnt < 4)
        {
            u8DesSeqTimeCnt++;
        }
        else if (u8DesSeqTimeCnt == 4)
        {
            u8DesSeqTimeCnt = 4;

            if (u8DesSeqCfgCnt < (sizeof(Lx07_au8DesSeq) / sizeof(Lx07_au8DesSeq[0])))
            {
                Ex_MstWriteBuffer(I2C0_ID, (uint8_t *)&Lx07_au8DesSeq[u8DesSeqCfgCnt].u16DstRegAddr, (uint8_t *)&Lx07_au8DesSeq[u8DesSeqCfgCnt].u8RegData);
                u8DesSeqCfgCnt++;
            }
            else if (u8DesSeqCfgCnt == (sizeof(Lx07_au8DesSeq) / sizeof(Lx07_au8DesSeq[0])))
            {
                u8DesSeqCfgCnt = (sizeof(Lx07_au8DesSeq) / sizeof(Lx07_au8DesSeq[0])) + 1;
                I2C_Disable(I2C0_ID);
                I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                I2C_Enable(I2C0_ID);
                boRte = true;
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return boRte;
}

static bool Lx07_vBklCfg(void)
{
    /*通道mix调光 + 100%亮度*/
    // 1.BKL_EN DES_GPIO_LOW(13);

    // 2.配置 MODE_CTRL 寄存器（地址 00h，default:0x0F）：0x8F(Mix调光)      0xAF是PWM调光

    // 3.IN_I2CDIM_H（地址05h，default:0）：MSB 6bit:0x1F     IN_I2CDIM_L（地址06h，0）:LSB 8bit: 0xFF   per = val / 0x3FFF    0x1FFF:50%

    // 4.FUN_SET_0 寄存器（地址 01h，default:0x14）—— 设定调光频率与EMI优化 0x14      200HZ

    // 5.FUN_SET_1 寄存器（地址 02h，default:0x14）—— Mix 模式转换点与开关频率  0x14  默认展屏

    static uint8_t u8BklTimeCnt = 0;
    static uint8_t u8BklCfgCnt  = 0;

    static bool boRte = false;

    if (!boRte)
    {
        if (u8BklTimeCnt < 2)
        {
            u8BklTimeCnt++;
        }
        else if (u8BklTimeCnt == 2)
        {
            u8BklTimeCnt = 3;
            I2C_Disable(I2C0_ID);
            I2C_SetTargetAddr(I2C0_ID, DEV_BKL);
            I2C_Enable(I2C0_ID);
        }
        else if (u8BklTimeCnt < 4)
        {
            u8BklTimeCnt++;
        }
        else if (u8BklTimeCnt == 4)
        {
            u8BklTimeCnt = 4;

            if (u8BklCfgCnt < (sizeof(Lx07_au8BklCfg) / sizeof(Lx07_au8BklCfg[0])))
            {
                Lx07_MstWriteByte(I2C0_ID, (uint8_t *)&Lx07_au8BklCfg[u8BklCfgCnt].u8DstRegAddr, (uint8_t *)&Lx07_au8BklCfg[u8BklCfgCnt].u8RegData);
                u8BklCfgCnt++;
            }
            else if (u8BklCfgCnt == (sizeof(Lx07_au8BklCfg) / sizeof(Lx07_au8BklCfg[0])))
            {
                u8BklCfgCnt = (sizeof(Lx07_au8BklCfg) / sizeof(Lx07_au8BklCfg[0])) + 1;
                I2C_Disable(I2C0_ID);
                I2C_SetTargetAddr(I2C0_ID, DEV_DES);
                I2C_Enable(I2C0_ID);
                boRte = true;

                Lx07_InitAllEndFlg = true;

                uint8_t au8BklLel[13] = {0x6E, 0x32, 0x2E, 0x76, 0x61, 0x6C, 0x3D, 0x31, 0x30, 0x30, 0xFF, 0xFF, 0xFF};
                Uart_Transmit(au8BklLel, sizeof(au8BklLel));

                // uint8_t au8UartTxData[] = {0x74, 0x31, 0x34, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x45, 0x65, 0x70, 0x72, 0x6f, 0x6d, 0x20, 0x22, 0xff, 0xff, 0xff}; // Eeprom
                // Uart_Transmit(au8UartTxData, sizeof(au8UartTxData));
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return boRte;
}
/*****************************************************************************
 * End file Lx07.c
 *****************************************************************************/
