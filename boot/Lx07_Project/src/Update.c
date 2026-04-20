/*****************************************************************************
 * @file Update.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2026-04-17
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "Update.h"
#include "Z20K11xM_regfile.h"
#include "Uart2.h"
#include "FlashDriver.h"
#include "Config.h"
#include "string.h"
#include "Uart2.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define REGFILE_ADDR (0x00)
#define REGFILE_DATA (0x20260417) //
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
Update_stInfo stUpdateInfo = {0};
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/

/*****************************************************************************
 * function definitions
 *****************************************************************************/
void Uart2_vReadInt(void)
{
    uint8_t u8Data = UART_ReceiveByte(UART2_ID);

    switch (stUpdateInfo.enCurSts)
    {
        case UPDATE_IDLE:
            stUpdateInfo.au8RecBuffer[stUpdateInfo.u8RecCount++] = u8Data;
            if (stUpdateInfo.u8RecCount >= 2)
                stUpdateInfo.u8RecCount = 0;
            break;
        case UPDATE_STEP3_FIRST_FRAME:
            stUpdateInfo.au8RecBuffer[stUpdateInfo.u8RecCount++] = u8Data;
            if (stUpdateInfo.u8RecCount >= 2)
                stUpdateInfo.u8RecCount = 0;
            break;
        case UPDATE_STEP4_WRITE_DATA:
            if (!stUpdateInfo.boWriteFlashFlg)
            {
                stUpdateInfo.au8UpgrateBuffer[stUpdateInfo.u16UpgrateCount++] = u8Data;
                if (stUpdateInfo.u16UpgrateCount >= UPDATE_DATA_SIZE)
                {
                    stUpdateInfo.u16UpgrateCount = 0;
                    stUpdateInfo.boWriteFlashFlg = true; // 此时准备开始写入Flash
                }

                if (4 == stUpdateInfo.u16UpgrateCount) // 判断主机发送的是不是最后一帧
                {
                    if ((0x66 == stUpdateInfo.au8UpgrateBuffer[0]) && (0x77 == stUpdateInfo.au8UpgrateBuffer[1]) && (0x88 == stUpdateInfo.au8UpgrateBuffer[2]) && (0x99 == stUpdateInfo.au8UpgrateBuffer[3]))
                    {
                        stUpdateInfo.u16UpgrateCount = 0;
                        stUpdateInfo.enCurSts        = UPDATE_STEP5_LAST_FRAME;
                    }
                }
            }
            break;
        default:
            break;
    }
}

void Update_vInit(void)
{
}

void Update_vHandle(void)
{
    switch (stUpdateInfo.enCurSts)
    {
        case UPDATE_IDLE:
        {
            uint32_t u32Data = 0;
            // REGFILE_ReadByRegID(REGFILE_ADDR, &u32Data);

            // if (REGFILE_DATA == u32Data)
            if ((0x12 == stUpdateInfo.au8RecBuffer[0]) && (0x34 == stUpdateInfo.au8RecBuffer[1])) // 收到上位机的进入升级标志
            {
                stUpdateInfo.enCurSts = UPDATE_STEP1_START;

                stUpdateInfo.u8RecCount = 0;
                memset(stUpdateInfo.au8RecBuffer, 0, sizeof(stUpdateInfo.au8RecBuffer));
            }
            else
            {
                // to app;
            }
            break;
        }
        case UPDATE_STEP1_START: // reserve
            stUpdateInfo.enCurSts = UPDATE_STEP2_ERASE_FLASH;
            break;
        case UPDATE_STEP2_ERASE_FLASH:
        {
            static uint8_t u8SectorCount = 0;

            if (u8SectorCount < (APP_SIZE / SECTOR_SIZE))
            {
                if (FlashDrive_boEraseSector(APP_START_ADDR + u8SectorCount * SECTOR_SIZE)) // erase 0x2000 8KB
                {
                    u8SectorCount++;
                }
            }

            if (u8SectorCount >= (APP_SIZE / SECTOR_SIZE))
            {
                stUpdateInfo.au8SendBuffer[0] = 0xA5;
                stUpdateInfo.au8SendBuffer[1] = 0xB5;
                Uart2_vSend(stUpdateInfo.au8SendBuffer, 2); // 擦除完成通知上位机
                stUpdateInfo.enCurSts = UPDATE_STEP3_FIRST_FRAME;
            }
            break;
        }
        case UPDATE_STEP3_FIRST_FRAME:
        {
            if ((0xC5 == stUpdateInfo.au8RecBuffer[0]) && (0xD5 == stUpdateInfo.au8RecBuffer[1])) // 收到上位机的第一帧
            {
                stUpdateInfo.enCurSts = UPDATE_STEP4_WRITE_DATA;

                stUpdateInfo.u8RecCount = 0;
                memset(stUpdateInfo.au8RecBuffer, 0, sizeof(stUpdateInfo.au8RecBuffer));

                stUpdateInfo.au8SendBuffer[0] = 0x22;
                stUpdateInfo.au8SendBuffer[1] = 0x22;
                Uart2_vSend(stUpdateInfo.au8SendBuffer, 2); // 通知上位机已收到数据
            }
            break;
        }
        case UPDATE_STEP4_WRITE_DATA:
        {
            static uint32_t u32FlashOffset = 0;

            if (stUpdateInfo.boWriteFlashFlg)
            {
                uint16_t u16Loop2 = 0;

                while (u16Loop2 < UPDATE_DATA_SIZE)
                {
                    if (FlashDrive_boProgramPhrase((APP_START_ADDR + u16Loop2 + u32FlashOffset), (uint8_t *)(&stUpdateInfo.au8UpgrateBuffer[0 + u16Loop2])))
                        u16Loop2 += 16;
                }
                // 每次写完偏移 +512
                u32FlashOffset += UPDATE_DATA_SIZE;

                stUpdateInfo.boWriteFlashFlg  = false;
                stUpdateInfo.au8SendBuffer[0] = 0x22;
                stUpdateInfo.au8SendBuffer[1] = 0x22;
                Uart2_vSend(stUpdateInfo.au8SendBuffer, 2); // 通知上位机可以发送下帧数据
            }
            break;
        }
        case UPDATE_STEP5_LAST_FRAME:
            // memset(&stUpdateInfo, 0, sizeof(stUpdateInfo));
            stUpdateInfo.enCurSts         = UPDATE_SUCCESS;
            stUpdateInfo.au8SendBuffer[0] = 0x99;
            stUpdateInfo.au8SendBuffer[1] = 0x99;
            Uart2_vSend(stUpdateInfo.au8SendBuffer, 2); // 通知上位机升级结束
            // to app
            break;
        case UPDATE_SUCCESS:
        {
            uint8_t au8BootValid[16] = {0xA5, 0xA5, 0xA5, 0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            if (FlashDrive_boEraseSector(DFLASH_START)) // erase 0x2000 8KB
            {
                if (FlashDrive_boProgramPhrase(DFLASH_START, au8BootValid))
                    NVIC_SystemReset();
            }
            // REGFILE_WriteByRegID(REGFILE_ADDR, 0);
            // Update_vJumpApp();
            break;
        }
        case UPDATE_FAIL:
            // REGFILE_WriteByRegID(REGFILE_ADDR, 0);
            // Update_vJumpApp();
            break;
        default:
            break;
    }
}

void Update_vJumpApp(void)
{
    uint32_t jump_addr = APP_START_ADDR;

    // 只读取 Reset_Handler 地址（不读 MSP）
    uint32_t reset_handler_addr = *(volatile uint32_t *)(jump_addr + 4);

    // // 安全检查（可选）
    // if ((reset_handler_addr & 0x01U) == 0 || reset_handler_addr < jump_addr)
    // {
    //     // 非法 → 进入升级模式
    //     Enter_Upgrade_Mode();
    //     return 0; // 或 while(1)
    // }

    // 跳转前强制 VTOR = 0
    SCB->VTOR = 0x00000000UL;

    // 清除 pending 中断
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    // 关中断（可选）
    __disable_irq();

    // 直接跳转到 App 的 Reset_Handler
    ((void (*)(void))reset_handler_addr)();
}
/*****************************************************************************
 * End file Update.c
 *****************************************************************************/
