/*****************************************************************************
 * @file Debounce.h
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2025-10-27
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/
#ifndef DEBOUNCE_H
#define DEBOUNCE_H

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
/*****************************************************************************
 * Global macros
 *****************************************************************************/

/*****************************************************************************
 * Global data types
 *****************************************************************************/
typedef struct
{
    bool     boFirTriggerFlg;
    bool     boValidFlg;
    uint16_t u16CheckCnt;  // 防抖检查计数器（由外部任务更新）
    uint16_t u16StableCnt; // 稳定触发所需的计数阈值（如5=50ms，10ms/次检查）
} Debounce_StInfo;
/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
extern Debounce_StInfo Debounce_StTpTrigger;
extern Debounce_StInfo Debounce_StSw1;
extern Debounce_StInfo Debounce_StSw2;
extern Debounce_StInfo Debounce_StSw3;
/*****************************************************************************
 * Global function prototypes
 *****************************************************************************/
void Debounce_vInit(Debounce_StInfo *pstDeb, uint16_t u16StableTicks);
void Debounce_vInIRQ(Debounce_StInfo *pstDeb);
void Debounce_vInTpIRQ(Debounce_StInfo *pstDeb); // 读取坐标(第二次进入中断),中断调用
void Debounce_vCheck(Debounce_StInfo *pstDeb);
bool Debounce_boGetValidStu(Debounce_StInfo *pstDeb);
#endif
/*****************************************************************************
 * End file DEBOUNCE_H
 *****************************************************************************/
