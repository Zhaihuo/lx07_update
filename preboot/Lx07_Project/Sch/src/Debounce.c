/*****************************************************************************
 * @file Debounce.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-10-27
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "Debounce.h"
#include "Touch.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/

/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
Debounce_StInfo Debounce_StTpTrigger = {0};
Debounce_StInfo Debounce_StSw1       = {0}; // 按键1
Debounce_StInfo Debounce_StSw2       = {0}; // 按键2
Debounce_StInfo Debounce_StSw3       = {0}; // 按键3

static uint8_t u8DebounceTpSecondCnt = 0; // 进入中断后第二次触发读到的才是当前坐标
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/

/*****************************************************************************
 * function definitions
 *****************************************************************************/
/**
 * @brief 初始化中断防抖参数
 * @param pstDeb 结构体指针
 * @param u16StableTicks 稳定触发所需的检查次数（如5=50ms，10ms检查一次）
 */
void Debounce_vInit(Debounce_StInfo *pstDeb, uint16_t u16StableTicks)
{
    if (NULL != pstDeb)
    {
        pstDeb->boFirTriggerFlg = true;
        pstDeb->boValidFlg      = false;
        pstDeb->u16CheckCnt     = 0;
        pstDeb->u16StableCnt    = u16StableTicks;
    }
    else
    {
        /*Nothing*/
    }
}

/**
 * @brief 中断服务程序中调用，标记中断触发
 * @param pstDeb 结构体指针
 */
void Debounce_vInIRQ(Debounce_StInfo *pstDeb)
{
    if (NULL != pstDeb)
    {
        if (pstDeb->boFirTriggerFlg) // 首次触发，判断直接有效
        {
            pstDeb->boFirTriggerFlg = false;
            pstDeb->boValidFlg      = true;
        }
        else
        {

            if (pstDeb->u16CheckCnt >= pstDeb->u16StableCnt)
            {
                pstDeb->boValidFlg = true;
            }
            else
            {
                /*Nothing*/
            }

            pstDeb->u16CheckCnt = 0; // 开始计数
        }
    }
    else
    {
        /*Nothing*/
    }
}

/**
 * @brief 中断服务程序中调用，读取触摸坐标
 * @param pstDeb 结构体指针
 */
void Debounce_vInTpIRQ(Debounce_StInfo *pstDeb)
{
    if (NULL != pstDeb)
    {
        if (pstDeb->boFirTriggerFlg) // 首次触发，判断直接有效
        {
            pstDeb->boFirTriggerFlg = false;
            pstDeb->boValidFlg      = true;
            Touch_u32TpCount++;
        }
        else
        {

            if (0 != u8DebounceTpSecondCnt)
            {
                u8DebounceTpSecondCnt = 0;
                pstDeb->boValidFlg    = true;
                Touch_u32TpCount++;
            }
            else
            {
                /*Nothing*/
            }

            if (pstDeb->u16CheckCnt >= pstDeb->u16StableCnt)
            {
                u8DebounceTpSecondCnt = 1;
                pstDeb->boValidFlg    = true;
            }
            else
            {
                /*Nothing*/
            }

            pstDeb->u16CheckCnt = 0; // 开始计数
        }
    }
    else
    {
        /*Nothing*/
    }
}

/**
 * @brief 外部任务（如主循环）定时调用，检查是否稳定触发
 * @param pstDeb 结构体指针
 * @param boCurState 外部任务读取的当前硬件状态（如按键电平）
 * @return true=稳定触发（有效中断），false=未稳定/已释放
 */
void Debounce_vCheck(Debounce_StInfo *pstDeb)
{
    if (NULL != pstDeb)
    {
        if (pstDeb->boFirTriggerFlg)
        {
            /*Nothing*/
        }
        else
        {
            pstDeb->u16CheckCnt++;
            if (pstDeb->u16CheckCnt >= pstDeb->u16StableCnt)
            {
                pstDeb->u16CheckCnt = pstDeb->u16StableCnt;
            }
            else
            {
                /*Nothing*/
            }
        }
    }
    else
    {
        /*Nothing*/
    }
}

bool Debounce_boGetValidStu(Debounce_StInfo *pstDeb)
{
    if (NULL != pstDeb)
    {
        if (pstDeb->boValidFlg)
        {
            pstDeb->boValidFlg = false;
            return true;
        }
        else
        {
            /*Nothing*/
        }
    }
    else
    {
        /*Nothing*/
    }
    return false;
}

/*****************************************************************************
 * End file Debounce.c
 *****************************************************************************/
