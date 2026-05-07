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
#include "Z20K11xM_stim.h"
#include "Z20k118M.h"
#include "Z20K11xM_drv.h"
#include "Z20K11xM_uart.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/

/*****************************************************************************
 * Local data types
 *****************************************************************************/
typedef enum
{
    UPDATE_APP,
    UPDATE_BOOT
} Update_enType;

typedef struct
{
    uint32_t      u32StartAddr;
    uint32_t      u32Size;
    Update_enType enUpdType;
} Update_stAddr;

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
static uint32_t u32RegID00Val = 0;
static bool     boJumpAppFlg  = false;

Update_stInfo stUpdateInfo    = {0};
Update_stAddr stUpdateAddrMsg = {0};
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static Update_stAddr Update_stGetAddrMsg(Update_enType enType, uint32_t u32Addr);
/*****************************************************************************
 * function definitions
 *****************************************************************************/
void Uart2_vReadInt(void)
{
    uint8_t u8Data = UART_ReceiveByte(UART2_ID);

    switch (stUpdateInfo.enCurSts)
    {
        case UPDATE_STEP1_START:
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
    SYSCTRL_ResetModule(SYSCTRL_REGFILE);
    SYSCTRL_EnableModule(SYSCTRL_REGFILE);
}

void Update_vDeInit(void)
{
    __disable_irq();

    /*Reset update flag*/
    uint32_t u32WrRegID00ValInit = 0x00;
    REGFILE_WriteByRegID(REGFILE_ID_00, &u32WrRegID00ValInit);

    /*DeInit uart2*/
    UART_IntMask(UART2_ID, UART_INT_ALL, MASK);
    // UART_EmptyRxFifo(UART2_ID);
    // SYSCTRL_ResetModule(SYSCTRL_UART2);
    // SYSCTRL_DisableModule(SYSCTRL_UART2);
    NVIC_DisableIRQ(UART2_IRQn);
    NVIC_ClearPendingIRQ(UART2_IRQn);

    /*DeInit stim0*/
    STIM_Disable(STIM_0);
    STIM_IntCmd(STIM_0, DISABLE);
    SYSCTRL_ResetModule(SYSCTRL_STIM);
    SYSCTRL_DisableModule(SYSCTRL_STIM);
    NVIC_DisableIRQ(STIM_IRQn);
    NVIC_ClearPendingIRQ(STIM_IRQn);

    /*DeInit flash*/
    SYSCTRL_ResetModule(SYSCTRL_FLASH);
    SYSCTRL_DisableModule(SYSCTRL_FLASH);
    NVIC_DisableIRQ(FLASH_IRQn);

    /*DeInit regfile*/
    SYSCTRL_ResetModule(SYSCTRL_REGFILE);
    SYSCTRL_DisableModule(SYSCTRL_REGFILE);

    /*CLose SysTick*/
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // NVIC->ICER[0] = 0xFFFFFFFF;
    // NVIC->ICPR[0] = 0xFFFFFFFF;
}

void Update_vHandle(void) // 10ms
{
    uint8_t  au8BootValid[16] = {0};
    uint32_t u32BootValue     = *(volatile uint32_t *)DFLASH_BOOT_STATUS_ADDR; /*从地址读取数据*/

    switch (stUpdateInfo.enCurSts)
    {
        case UPDATE_IDLE:
        {
            if ((BOOT_A_VALID != u32BootValue) && (BOOT_B_VALID != u32BootValue)) // 首次刷写，默认boot_A有效
            {
                au8BootValid[0] = (BOOT_A_VALID >> 24u) & 0xff;
                au8BootValid[1] = (BOOT_A_VALID >> 16u) & 0xff;
                au8BootValid[2] = (BOOT_A_VALID >> 8u) & 0xff;
                au8BootValid[3] = (BOOT_A_VALID >> 0u) & 0xff;

                if (FlashDrive_boEraseSector(DFLASH_BOOT_STATUS_ADDR))
                {
                    if (FlashDrive_boProgramPhrase(DFLASH_BOOT_STATUS_ADDR, au8BootValid))
                    {
                        REGFILE_ReadByRegID(REGFILE_ID_00, &u32RegID00Val);
                        stUpdateInfo.enCurSts = UPDATE_STEP1_START;
                    }
                }
            }
            else
            {
                REGFILE_ReadByRegID(REGFILE_ID_00, &u32RegID00Val);
                stUpdateInfo.enCurSts = UPDATE_STEP1_START;
            }
            break;
        }
        case UPDATE_STEP1_START:
        {
            if (REGFILE_UPDATE_APP_FLG == u32RegID00Val)
            {
                static uint32_t u32WrRegID00Val = 0x00;
                if (0x00 == u32WrRegID00Val) // 写一次标志位
                {
                    REGFILE_WriteByRegID(REGFILE_ID_00, &u32WrRegID00Val);
                    u32WrRegID00Val++;
                }
                ELSE_NOTHING;

                if ((0x12 == stUpdateInfo.au8RecBuffer[0]) && (0x34 == stUpdateInfo.au8RecBuffer[1])) // 收到上位机的进入升级标志
                {
                    Update_stGetAddrMsg(UPDATE_APP, APP_A_START_ADDR);
                    stUpdateInfo.enCurSts = UPDATE_STEP2_ERASE_FLASH;

                    stUpdateInfo.u8RecCount = 0;
                    memset(stUpdateInfo.au8RecBuffer, 0, sizeof(stUpdateInfo.au8RecBuffer));
                }
                else // 进入boot后，10s收不到升级指令会复位
                {
                    static uint16_t u16TimeCt = 0;
                    u16TimeCt++;
                    if (u16TimeCt >= (10000 / 10))
                    {
                        u16TimeCt = 0;
                        NVIC_SystemReset();
                    }
                    ELSE_NOTHING;
                }
            }
            else if (REGFILE_UPDATE_BOOT_FLG == u32RegID00Val)
            {
                static uint32_t u32WrRegID00Val = 0x00;
                if (0x00 == u32WrRegID00Val) // 写一次标志位
                {
                    REGFILE_WriteByRegID(REGFILE_ID_00, &u32WrRegID00Val);
                    u32WrRegID00Val++;
                }
                ELSE_NOTHING;

                if ((0x12 == stUpdateInfo.au8RecBuffer[0]) && (0x34 == stUpdateInfo.au8RecBuffer[1])) // 收到上位机的进入升级标志
                {
                    if (BOOT_A_VALID == u32BootValue)
                    {
                        Update_stGetAddrMsg(UPDATE_BOOT, BOOT_B_START_ADDR);
                    }
                    else if (BOOT_B_VALID == u32BootValue)
                    {
                        Update_stGetAddrMsg(UPDATE_BOOT, BOOT_A_START_ADDR);
                    }
                    else /*升级异常*/
                    {
                        NVIC_SystemReset();
                    }
                    stUpdateInfo.enCurSts = UPDATE_STEP2_ERASE_FLASH;

                    stUpdateInfo.u8RecCount = 0;
                    memset(stUpdateInfo.au8RecBuffer, 0, sizeof(stUpdateInfo.au8RecBuffer));
                }
                else // 进入boot后，10s收不到升级指令会复位
                {
                    static uint16_t u16TimeCt = 0;
                    u16TimeCt++;
                    if (u16TimeCt >= (10000 / 10))
                    {
                        u16TimeCt = 0;
                        NVIC_SystemReset();
                    }
                    ELSE_NOTHING;
                }
            }
            else
            {
                boJumpAppFlg = true;
            }
            break;
        }
        case UPDATE_STEP2_ERASE_FLASH:
        {
            static uint8_t u8SectorCount = 0;

            if (u8SectorCount < (stUpdateAddrMsg.u32Size / SECTOR_SIZE))
            {
                if (FlashDrive_boEraseSector(stUpdateAddrMsg.u32StartAddr + u8SectorCount * SECTOR_SIZE)) // erase 0x2000 8KB
                {
                    u8SectorCount++;
                }
            }

            if (u8SectorCount >= (stUpdateAddrMsg.u32Size / SECTOR_SIZE))
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
                    if (FlashDrive_boProgramPhrase((stUpdateAddrMsg.u32StartAddr + u16Loop2 + u32FlashOffset), (uint8_t *)(&stUpdateInfo.au8UpgrateBuffer[0 + u16Loop2])))
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
        {
            static uint8_t u8DelagCt = 0;

            if (0 == u8DelagCt)
            {
                // memset(&stUpdateInfo, 0, sizeof(stUpdateInfo));
                stUpdateInfo.au8SendBuffer[0] = 0x99;
                stUpdateInfo.au8SendBuffer[1] = 0x99;
                Uart2_vSend(stUpdateInfo.au8SendBuffer, 2); // 通知上位机升级结束
            }
            else if (u8DelagCt >= 10) // 确保串口发送成功
            {
                stUpdateInfo.enCurSts = UPDATE_SUCCESS;
            }
            ELSE_NOTHING;

            u8DelagCt++;
            break;
        }
        case UPDATE_SUCCESS:
        {
            if (UPDATE_APP == stUpdateAddrMsg.enUpdType)
            {
                boJumpAppFlg = true;
            }
            else
            {
                if (BOOT_A_VALID == u32BootValue)
                {
                    au8BootValid[0] = (BOOT_B_VALID >> 24u) & 0xff;
                    au8BootValid[1] = (BOOT_B_VALID >> 16u) & 0xff;
                    au8BootValid[2] = (BOOT_B_VALID >> 8u) & 0xff;
                    au8BootValid[3] = (BOOT_B_VALID >> 0u) & 0xff;
                }
                else if (BOOT_B_VALID == u32BootValue)
                {
                    au8BootValid[0] = (BOOT_A_VALID >> 24u) & 0xff;
                    au8BootValid[1] = (BOOT_A_VALID >> 16u) & 0xff;
                    au8BootValid[2] = (BOOT_A_VALID >> 8u) & 0xff;
                    au8BootValid[3] = (BOOT_A_VALID >> 0u) & 0xff;
                }
                else /*升级异常*/
                {
                    NVIC_SystemReset();
                }

                if (FlashDrive_boEraseSector(DFLASH_BOOT_STATUS_ADDR))
                {
                    if (FlashDrive_boProgramPhrase(DFLASH_BOOT_STATUS_ADDR, au8BootValid))
                        boJumpAppFlg = true;
                }
            }
            break;
        }
        case UPDATE_FAIL:
            break;
        default:
            break;
    }
}

static Update_stAddr Update_stGetAddrMsg(Update_enType enType, uint32_t u32Addr)
{
    if (UPDATE_APP == enType)
    {
        stUpdateAddrMsg.u32StartAddr = u32Addr;
        stUpdateAddrMsg.u32Size      = APP_SIZE;
    }
    else
    {
        stUpdateAddrMsg.u32StartAddr = u32Addr;
        stUpdateAddrMsg.u32Size      = BOOT_SIZE;
    }
    stUpdateAddrMsg.enUpdType = enType;

    return stUpdateAddrMsg;
}

void Update_vJumpApp(void)
{
    Update_vDeInit();

    typedef void (*app_entry_t)(void);
    uint32_t    jump_addr         = APP_A_START_ADDR;
    uint32_t    app_reset_handler = *(volatile uint32_t *)(jump_addr + 4);
    app_entry_t app_entry         = (app_entry_t)app_reset_handler;

    /* Cortex‑M0+*/
    __DSB();
    __ISB();
    app_entry();

    while (true);
}

bool Update_boJumpAppFlag(void)
{
    return boJumpAppFlg;
}
/*****************************************************************************
 * End file Update.c
 *****************************************************************************/
