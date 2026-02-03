/*****************************************************************************
 * @file Adc.c
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
#include "Adc.h"
#include "stdint.h"
#include "i2c.h"
#include "ProductLine.h"
#include "uart.h"
#include "Uart2.h"
#include "Config.h"
#include "can.h"
#include "BackL.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define ADC_ABS(a, b) ((a > b) ? (a - b) : (b - a))

#define ADC_TMEP_OFFSET_PCB (40)
#define ADC_TMEP_OFFSET_LCD (55)

#define ADC_INVALID_TEMP (-999.0f) // 无效温度标识
/*****************************************************************************
 * Local data types
 *****************************************************************************/
// 电阻-温度映射表（center值）：{温度(℃), 电阻(kΩ)}，按温度升序排列
typedef struct
{
    int   temp; // 温度基准点（每5℃一个基准）
    float res;  // 对应温度的电阻center值
} Adc_stResTempRef;

typedef enum
{
    ADC_PCB_TEMP,
    ADC_LCD_TEMP,
} Adc_enTempTyp;
/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
// 基准数据：温度从-40℃到125℃，每5℃一个点（共34个基准点）  //  NCP15XH103F03RC
static const Adc_stResTempRef stAdPcbResTempRefs[] = {
    {-40, 195.6520},
    {-35, 148.1710},
    {-30, 113.3471},
    {-25, 87.5588 },
    {-20, 68.2367 },
    {-15, 53.6496 },
    {-10, 42.5062 },
    {-5,  33.8922 },
    {0,   27.2186 },
    {5,   22.0211 },
    {10,  17.9255 },
    {15,  14.6735 },
    {20,  12.0805 },
    {25,  10.0000 },
    {30,  8.3145  },
    {35,  6.9479  },
    {40,  5.8336  },
    {45,  4.9169  },
    {50,  4.1609  },
    {55,  3.5350  },
    {60,  3.0143  },
    {65,  2.5861  },
    {70,  2.2275  },
    {75,  1.9245  },
    {80,  1.6685  },
    {85,  1.4521  },
    {90,  1.2680  },
    {95,  1.0965  },
    {100, 0.9738  },
    {105, 0.8798  },
    {110, 0.8049  },
    {115, 0.7305  },
    {120, 0.6591  },
    {125, 0.5821  }
};

// 基准数据：温度从-55℃到150℃，每5℃一个点（共42个基准点）   //NCU15XH103F6SRC
static const Adc_stResTempRef stAdLcdResTempRefs[] = {
    {-55, 470.1223},
    {-50, 348.6184},
    {-45, 260.2623},
    {-40, 195.6520},
    {-35, 148.1710},
    {-30, 113.3471},
    {-25, 87.5588 },
    {-20, 68.2367 },
    {-15, 53.6496 },
    {-10, 42.5062 },
    {-5,  33.8922 },
    {0,   27.2186 },
    {5,   22.0211 },
    {10,  17.9255 },
    {15,  14.6735 },
    {20,  12.0805 },
    {25,  10.0000 },
    {30,  8.3145  },
    {35,  6.9479  },
    {40,  5.8336  },
    {45,  4.9169  },
    {50,  4.1609  },
    {55,  3.5350  },
    {60,  3.0143  },
    {65,  2.5861  },
    {70,  2.2275  },
    {75,  1.9245  },
    {80,  1.6685  },
    {85,  1.4521  },
    {90,  1.2680  },
    {95,  1.0965  },
    {100, 0.9738  },
    {105, 0.8798  },
    {110, 0.8049  },
    {115, 0.7305  },
    {120, 0.6591  },
    {125, 0.5821  },
    {130, 0.4742  },
    {135, 0.4335  },
    {140, 0.3897  },
    {145, 0.3647  },
    {150, 0.3095  }
};
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static void  Adc_vFindResSegment(float res, int *left, int *right, const Adc_stResTempRef astResTempRef[], Adc_enTempTyp enTempTyp);
static float Adc_flCalcTemperature(float res, const Adc_stResTempRef astResTempRef[], Adc_enTempTyp enTempTyp);
/*****************************************************************************
 * function definitions
 *****************************************************************************/
void Adc_vInit(void)
{
    ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_AdHw] = true;
    // ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_Batt] = true;
}

void Adc_vHandle(void) // 5ms
{
    static uint16_t u16TrigCnt = 1;

    u16TrigCnt++;

    if (u16TrigCnt % MAIN_TIME_MS(1000) == 0)
    {
        // if (!ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_Batt])
        //     ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_Batt] = true;
    }
    else
    {}

    if (u16TrigCnt % MAIN_TIME_MS(1155) == 0)
    {
        // if (!ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_LcdTemp])
        //     ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_LcdTemp] = true;
    }
    else
    {}

    if (u16TrigCnt % MAIN_TIME_MS(1355) == 0)
    {
        // if (!ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_PcbTemp])
        //     ProductLine_stI2cRWMsgs.Read_t.aboStrtFlg[ProD_I2c_Read_PcbTemp] = true;
    }
    else
    {}

    if (u16TrigCnt >= MAIN_TIME_MS(5000))
    {
        u16TrigCnt = 1;
    }
    else
    {}
}

void Adc_vReadLcdTemp(void) // NCU15XH103F6SRC
{
    static uint8_t u8StepLcbT = 3;

    uint8_t  u8ConfigArray[] = {0x01u, 0xd3, 0xc3};
    uint16_t u16ReadAd       = 0;
    double   dbAdVol, dbRes = 0.0;
    uint8_t  u8Temp          = 0;
    float    flTemp          = 0.0;
    uint8_t  au8UartTxData[] = {0x74, 0x37, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x20, 0x32, 0x35, 0x22, 0xff, 0xff, 0xff};

    if (3 == u8StepLcbT)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x49);
        I2C_Enable(I2C0_ID);
        u8StepLcbT = 2;
    }
    else if (2 == u8StepLcbT)
    {
        Ex_MstWriteDoubleByte(I2C0_ID, (uint8_t *)(u8ConfigArray));
        u8StepLcbT = 1;
    }
    else if (1 == u8StepLcbT)
    {
        uint8_t addr_ad = 0;
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&addr_ad));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_LcdTemp] = true;

        u8StepLcbT = 0;
    }
    else if (0 == u8StepLcbT)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_LcdTemp])
        {
            u8StepLcbT = 3;

            ProductLine_enCurWorkSts = ProDWork_Idle;

            u16ReadAd = (ProductLine_stI2cRWMsgs.Read_t.au8Data[0] << 4u) | (ProductLine_stI2cRWMsgs.Read_t.au8Data[1] >> 4u);
            dbAdVol   = (u16ReadAd * 4.096) / 0x7FF;

            /*R = 10 * V_ad / (3.3 - V_ad)*/
            if (dbAdVol >= 3.3)
            {
                // 理论上采集的电压不可能大于3.3v
            }
            else
            {
                dbRes  = 10 * dbAdVol / (3.3 - dbAdVol);
                flTemp = Adc_flCalcTemperature((float)dbRes, stAdLcdResTempRefs, ADC_LCD_TEMP);
            }

            if (flTemp >= 0)
            {
                u8Temp           = (uint8_t)(flTemp + 0.5);
                au8UartTxData[8] = 0x20; // 空格

                BackL_vDireating(u8Temp);
            }
            else
            {
                u8Temp           = (uint8_t)((flTemp - 0.5) * (-1));
                au8UartTxData[8] = 0x2d; // 负号
            }

            if (ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_LcdTemp])
            {
                /*Tx 0x500*/
                memset(stCanRxBuf.data, 0, 8);

                if (flTemp >= 0)
                {
                    stCanRxBuf.data[4] = u8Temp + ADC_TMEP_OFFSET_LCD;
                }
                else
                {
                    stCanRxBuf.data[4] = ADC_TMEP_OFFSET_LCD - u8Temp;
                }

                CAN_Send_Msg(0x500, stCanRxBuf.data);

                ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_LcdTemp] = false;
            }
            else
            {}

            au8UartTxData[9]  = 0x30 + u8Temp / 10;
            au8UartTxData[10] = 0x30 + (u8Temp % 10);

            Uart_Transmit(au8UartTxData, sizeof(au8UartTxData));
        }
    }
}

void Adc_vReadPcbTemp(void) // NCP15XH103F03RC
{
    static uint8_t u8StepPcbT = 3;

    uint8_t  u8ConfigArray[] = {0x01u, 0xf3, 0xc3};
    uint16_t u16ReadAd       = 0;
    double   dbAdVol, dbRes = 0.0;
    uint8_t  u8Temp          = 0;
    float    flTemp          = 0.0;
    uint8_t  au8UartTxData[] = {0x74, 0x31, 0x35, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x20, 0x33, 0x35, 0x22, 0xff, 0xff, 0xff};

    if (3 == u8StepPcbT)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x49);
        I2C_Enable(I2C0_ID);
        u8StepPcbT = 2;
    }
    else if (2 == u8StepPcbT)
    {
        Ex_MstWriteDoubleByte(I2C0_ID, (uint8_t *)(u8ConfigArray));
        u8StepPcbT = 1;
    }
    else if (1 == u8StepPcbT)
    {
        uint8_t addr_ad = 0;
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&addr_ad));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_PcbTemp] = true;

        u8StepPcbT = 0;
    }
    else if (0 == u8StepPcbT)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_PcbTemp])
        {
            u8StepPcbT = 3;

            ProductLine_enCurWorkSts = ProDWork_Idle;

            u16ReadAd = (ProductLine_stI2cRWMsgs.Read_t.au8Data[0] << 4u) | (ProductLine_stI2cRWMsgs.Read_t.au8Data[1] >> 4u);
            dbAdVol   = (u16ReadAd * 4.096) / 0x7FF;

            /*R = v_ad / (3.3 - V_ad)*/
            if (dbAdVol >= 3.3)
            {
                // 理论上采集的电压不可能大于3.3v
            }
            else
            {
                dbRes  = dbAdVol / (3.3 - dbAdVol);
                flTemp = Adc_flCalcTemperature((float)dbRes, stAdLcdResTempRefs, ADC_PCB_TEMP);
            }

            if (flTemp >= 0)
            {
                u8Temp           = (uint8_t)(flTemp + 0.5);
                au8UartTxData[9] = 0x20; // 空格
            }
            else
            {
                u8Temp           = (uint8_t)((flTemp - 0.5) * (-1));
                au8UartTxData[9] = 0x2d; // 负号
            }

            if (ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_PcbTemp])
            {
                /*Tx 0x500*/
                memset(stCanRxBuf.data, 0, 8);

                if (flTemp >= 0)
                {
                    stCanRxBuf.data[5] = u8Temp + ADC_TMEP_OFFSET_PCB;
                }
                else
                {
                    stCanRxBuf.data[5] = ADC_TMEP_OFFSET_PCB - u8Temp;
                }

                CAN_Send_Msg(0x500, stCanRxBuf.data);

                ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_PcbTemp] = false;
            }
            else
            {}

            au8UartTxData[10] = 0x30 + u8Temp / 10;
            au8UartTxData[11] = 0x30 + (u8Temp % 10);

            Uart_Transmit(au8UartTxData, sizeof(au8UartTxData));
        }
    }
}

void Adc_vReadBatt(void)
{
    static uint8_t u8StepBatt = 3;

    uint8_t  u8ConfigArray[] = {0x01u, 0xc3, 0xc3}; // batt
    uint16_t u16ReadAd       = 0;
    double   dbAdVol, dbBatt, dbCalcuBatt = 0.0;

    if (3 == u8StepBatt)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x49);
        I2C_Enable(I2C0_ID);
        u8StepBatt = 2;
    }
    else if (2 == u8StepBatt)
    {
        Ex_MstWriteDoubleByte(I2C0_ID, (uint8_t *)(u8ConfigArray));
        u8StepBatt = 1;
    }
    else if (1 == u8StepBatt)
    {
        uint8_t addr_ad = 0;
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&addr_ad));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Batt] = true;

        u8StepBatt = 0;
    }
    else if (0 == u8StepBatt)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_Batt])
        {
            u8StepBatt = 3;

            ProductLine_enCurWorkSts = ProDWork_Idle;

            u16ReadAd = (ProductLine_stI2cRWMsgs.Read_t.au8Data[0] << 4u) | (ProductLine_stI2cRWMsgs.Read_t.au8Data[1] >> 4u);

            dbAdVol = (u16ReadAd * 4.096) / 0x7FF;
            dbBatt  = dbAdVol / (20.0 / (150 + 20));

            dbCalcuBatt = dbBatt + 0.45;

            if (ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_Batt])
            {
                /*Tx 0x500*/
                memset(stCanRxBuf.data, 0, 8);

                dbCalcuBatt >= 25.5 ? dbCalcuBatt = 25.5 : dbCalcuBatt;

                stCanRxBuf.data[6] = (uint8_t)(dbCalcuBatt * 10);

                CAN_Send_Msg(0x500, stCanRxBuf.data);

                ProductLine_stI2cRWMsgs.Read_t.aboHaveTx[ProD_I2c_Read_Batt] = false;
            }
            else
            {}

            // UART_PRINTF("Ad_Val = %d, dbCalcuBatt = %.3f\r\n", u16ReadAd, dbCalcuBatt);
        }
    }
}

void Adc_vReadHw(void)
{
    static uint8_t u8StepHw = 3;

    uint8_t  u8ConfigArray[] = {0x01u, 0xe3, 0xc3};
    uint16_t u16ReadAd       = 0;
    double   dbAdVol         = 0.0;
    uint8_t  au8UartTxData[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x32, 0x2e, 0x30, 0x2e, 0x30, 0x22, 0xff, 0xff, 0xff};
    uint8_t  u8MinIndex      = 0;
    float    aflVad[5]       = {3.3, 3, 2.24, 1.65, 1.1};
    float    aflAbs[5]       = {0};

    if (3 == u8StepHw)
    {
        I2C_Disable(I2C0_ID);
        I2C_SetTargetAddr(I2C0_ID, 0x49);
        I2C_Enable(I2C0_ID);
        u8StepHw = 2;
    }
    else if (2 == u8StepHw)
    {
        Ex_MstWriteDoubleByte(I2C0_ID, (uint8_t *)(u8ConfigArray));
        u8StepHw = 1;
    }
    else if (1 == u8StepHw)
    {
        uint8_t addr_ad = 0;
        Ex_MstReadArray(I2C0_ID, (uint8_t *)(&addr_ad));
        ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_AdHw] = true;

        u8StepHw = 0;
    }
    else if (0 == u8StepHw)
    {
        if (!ProductLine_stI2cRWMsgs.Read_t.aboEndFlg[ProD_I2c_Read_AdHw])
        {
            u8StepHw = 3;

            ProductLine_enCurWorkSts = ProDWork_Idle;

            u16ReadAd = (ProductLine_stI2cRWMsgs.Read_t.au8Data[0] << 4u) | (ProductLine_stI2cRWMsgs.Read_t.au8Data[1] >> 4u);
            dbAdVol   = (u16ReadAd * 4.096) / 0x7FF;

            /*
                R = (33 / V_ad) - 10                理论计算V_ad
                R = 0;      Hw version = 1.0.0      3.3v
                R = 1k;     Hw version = 1.1.0      3v
                R = 4.7k;   Hw version = 1.2.0      2.24v
                R = 10k;    Hw version = 1.3.0      1.65v
                R = 20k;    Hw version = 1.4.0      1.1v
            */

            for (uint8_t i = 0; i < 5; i++)
            {
                aflAbs[i] = ADC_ABS(dbAdVol, aflVad[i]);
            }

            float flAbsMin = aflAbs[0];

            for (uint8_t j = 0; j < 5; j++)
            {
                if (aflAbs[j] < flAbsMin)
                {
                    flAbsMin   = aflAbs[j]; // 更新最小值
                    u8MinIndex = j;         // 更新最小ID
                }
            }

            switch (u8MinIndex)
            {
                case 0: // 1.0.0
                    au8UartTxData[10] = 0x30;
                    break;
                case 1: // 1.1.0
                    au8UartTxData[10] = 0x31;
                    break;
                case 2: // 1.2.0
                    au8UartTxData[10] = 0x32;
                    break;
                case 3: // 1.3.0
                    au8UartTxData[10] = 0x33;
                    break;
                case 4: // 1.4.0
                    au8UartTxData[10] = 0x34;
                    break;
                default:
                    break;
            }
            Uart_Transmit(au8UartTxData, sizeof(au8UartTxData));
        }
    }
}

/**
 * @brief 二分查找电阻所在的基准段
 * @param res 输入电阻值（kΩ）
 * @param left 输出段左基准索引
 * @param right 输出段右基准索引
 * @return true：找到有效段，false：电阻超出范围
 */
static void Adc_vFindResSegment(float res, int *left, int *right, const Adc_stResTempRef astResTempRef[], Adc_enTempTyp enTempTyp)
{
    uint8_t u8ArrSize = 0;

    if (ADC_PCB_TEMP == enTempTyp)
    {
        u8ArrSize = 34;
    }
    else
    {
        u8ArrSize = 42;
    }

    // 电阻特性：温度升高，电阻减小（基准数据按温度升序，电阻降序）
    if (res > astResTempRef[0].res)
    {
        res = astResTempRef[0].res;
    }
    else if (res < astResTempRef[u8ArrSize - 1].res)
    {
        res = astResTempRef[u8ArrSize - 1].res;
    }
    else
    {}

    // 二分查找（O(log n)效率，比线性遍历快）
    int low = 0, high = u8ArrSize - 1;
    while (low + 1 < high)
    {
        int mid = (low + high) / 2;
        if (res > astResTempRef[mid].res)
        {
            high = mid; // 电阻较大，对应温度较低，往左侧找
        }
        else
        {
            low = mid; // 电阻较小，对应温度较高，往右侧找
        }
    }

    *left  = low;
    *right = high;
}

/**
 * @brief 根据电阻值计算温度（分段线性插值）
 * @param res 输入电阻值（kΩ）
 * @return 计算出的温度（℃），无效返回INVALID_TEMP
 */
static float Adc_flCalcTemperature(float res, const Adc_stResTempRef astResTempRef[], Adc_enTempTyp enTempTyp)
{
    int left_idx, right_idx;

    // 1. 查找电阻所在的基准段
    Adc_vFindResSegment(res, &left_idx, &right_idx, astResTempRef, enTempTyp);

    // 2. 获取段内两个基准点的温度和电阻
    const Adc_stResTempRef *left  = &astResTempRef[left_idx];
    const Adc_stResTempRef *right = &astResTempRef[right_idx];

    // 3. 分段线性插值计算（T = T1 + (R - R1) * (T2 - T1)/(R2 - R1)）
    // 避免除以零（基准数据保证R1 != R2）
    float temp = left->temp + (res - left->res) * (right->temp - left->temp) / (right->res - left->res);

    return temp;
}
/*****************************************************************************
 * End file Adc.c
 *****************************************************************************/
