/*****************************************************************************
 * @file Update.h
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2026-04-17
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/
#ifndef UPDATE_H
#define UPDATE_H

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
/*****************************************************************************
 * Global macros
 *****************************************************************************/
#define UPDATE_DATA_SIZE (512)
/*****************************************************************************
 * Global data types
 *****************************************************************************/
typedef enum
{
    UPDATE_IDLE,
    UPDATE_STEP1_START,
    UPDATE_STEP2_ERASE_FLASH,
    UPDATE_STEP3_FIRST_FRAME,
    UPDATE_STEP4_WRITE_DATA,
    UPDATE_STEP5_LAST_FRAME,
    UPDATE_SUCCESS,
    UPDATE_FAIL,
} Update_enSts;

typedef struct
{
    Update_enSts enCurSts;

    uint8_t u8RecCount;
    uint8_t au8RecBuffer[8];

    uint8_t u8SendCount;
    uint8_t au8SendBuffer[8];

    uint16_t u16UpgrateCount;
    uint8_t  au8UpgrateBuffer[UPDATE_DATA_SIZE];

    bool boWriteFlashFlg;
} Update_stInfo;

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/

/*****************************************************************************
 * Global function prototypes
 *****************************************************************************/
void Update_vInit(void);
void Update_vHandle(void);
void Update_vJumpApp(void);
bool Update_boJumpAppFlag(void);
#endif
/*****************************************************************************
 * End file UPDATE_H
 *****************************************************************************/
