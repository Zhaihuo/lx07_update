/*****************************************************************************
 * @file FlashDriver.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2026-03-03
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "FlashDrive.h"

#include "Z20K11xM_drv.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_srmc.h"
#include "Z20K11xM_uart.h"
#include "Z20K11xM_gpio.h"
#include "Z20K11xM_flash.h"
#include "Z20K11xM_wdog.h"
#include "Z20K11xM_pmu.h"

/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define SECTOR_SIZE 0x2000

#define BOOT_A_VALID (0xA5A5A5A5u)
#define BOOT_B_VALID (0xB5B5B5B5u)
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/
FLASH_CmdConfig_t FlashDrive_CmdExeConfig =
    {
        FLASH_CMD_ACT_WAIT,
        NULL};

static volatile uint8_t guIntFlag;
uint32_t                PriMaskReg = 0x12341234U;
/* Interupt vector table*/
extern uint32_t __vector_table;
/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static ResultStatus_t FlashDrive_BoardInit(void);

static void FlashDrive_vExample(void);
static void FlashDrive_CCIFIntCallBack(void);
static void FlashDrive_SuspendInterrupt(void);
static void FlashDrive_ResumeInterrupt(void);
/*****************************************************************************
 * function definitions
 *****************************************************************************/
void FlashDrive_Init(void)
{
    ResultStatus_t Sts = FlashDrive_BoardInit();

    guIntFlag = 0;
    /* Flash command complete interrupt */
    FLASH_InstallCallBackFunc(FLASH_INT_CCIF, FlashDrive_CCIFIntCallBack);
    /* enables flash interrupt in the NVIC interrupt controller */
    NVIC_EnableIRQ(FLASH_IRQn);
}

void FlashDrive_Handler(void)
{
    // FlashDrive_vExample();

    uint32_t        u32Loop1 = 0;
    static uint32_t u32Loop2 = 0;

    // static bool boCheckValidFlg = false;

    volatile uint32_t *u32MetaData = (volatile uint32_t *)(0x0003E000);

    uint8_t au8EveryWriteData1[16] = {0xA5, 0xA5, 0xA5, 0xA5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t au8EveryWriteData2[16] = {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC};

    if ((BOOT_A_VALID == u32MetaData[1]) || (BOOT_B_VALID == u32MetaData[1]))
    {
    }
    else
    {
        bool boFirstWriteFlg = true;

        while (boFirstWriteFlg)
        {
            /*Erase 8k, the last 8k in the 256k flash, start address: 0x0003E000*/
            if (FLASH_EraseSector(0x0003E000 + (u32Loop1 * SECTOR_SIZE), &FlashDrive_CmdExeConfig) != SUCC)
            {
                if (0 == u32Loop2)
                {
                    if (FLASH_ProgramPhrase(0x0003E000 + u32Loop2, (uint8_t *)au8EveryWriteData1, &FlashDrive_CmdExeConfig) != SUCC)
                        u32Loop2 += 16;
                }
                else
                {
                    if (u32Loop2 >= SECTOR_SIZE)
                    {
                        /*Nothing*/
                    }
                    else
                    {
                        if (FLASH_ProgramPhrase(0x0003E000 + u32Loop2, (uint8_t *)au8EveryWriteData2, &FlashDrive_CmdExeConfig) != SUCC)
                            u32Loop2 += 16;

                        if (u32Loop2 >= SECTOR_SIZE)
                        {
                            u32Loop2        = SECTOR_SIZE;
                            boFirstWriteFlg = false;
                        }
                    }
                }
            }
        }
    }
}

static ResultStatus_t FlashDrive_BoardInit(void)
{
    /* Interrupt vector table redefinition*/
    SCB->VTOR = ((uint32_t)&__vector_table) & 0xFFFFFF00U;
    __DSB();

    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();

    return SUCC;
}

static void FlashDrive_vExample(void)
{
    uint32_t uLoop;

    /* start erase data flash, erase 8192 bytes every time*/
    for (uLoop = 0; uLoop < 16; uLoop++)
    {
        if (FLASH_EraseSector(0x01000000 + (uLoop * SECTOR_SIZE),
                              &FlashDrive_CmdExeConfig) != SUCC)
        {
            while (1);
        }
    }

    /* program data flash(from 0x01000000 to 0x0101ffff), program 16 bytes every time */
    uint8_t padValue[16];
    for (uLoop = 0; uLoop < 16; uLoop++)
    {
        padValue[uLoop] = 0x5A;
    }

    for (uLoop = 0; uLoop < (SECTOR_SIZE * 16); uLoop += 16)
    {
        if (FLASH_ProgramPhrase(0x01000000 + uLoop, (uint8_t *)padValue,
                                &FlashDrive_CmdExeConfig) != SUCC)
        {
            while (1);
        }
    }
    /*You can read the specific data flash address(from 0x01000000 to 0x0101ffff) to check the result. */

    /* start erase pflash, erase 8192 bytes every time*/
    FlashDrive_SuspendInterrupt();
    for (uLoop = 0; uLoop < 16; uLoop++)
    {
        if (FLASH_EraseSector(0x00010000 + (uLoop * SECTOR_SIZE),
                              &FlashDrive_CmdExeConfig) != SUCC)
        {
            while (1);
        }
    }
    FlashDrive_ResumeInterrupt();

    /* program pflash(from 0x00010000 to 0x0002ffff), program 16 bytes every time */
    FlashDrive_SuspendInterrupt();
    for (uLoop = 0; uLoop < (SECTOR_SIZE * 16); uLoop += 16)
    {
        if (FLASH_ProgramPhrase(0x00010000 + uLoop, (uint8_t *)padValue,
                                &FlashDrive_CmdExeConfig) != SUCC)
        {
            while (1);
        }
    }
    FlashDrive_ResumeInterrupt();
}

/* Flash command complete interrupt function */
static void FlashDrive_CCIFIntCallBack(void)
{
    guIntFlag = 1;
    FLASH_IntMask(FLASH_INT_CCIF, MASK);
}

static void FlashDrive_SuspendInterrupt(void)
{
    PriMaskReg = __get_PRIMASK();

    /* Suspend all interrupts */
    if (0U == PriMaskReg)
    {
        __disable_irq();
    }
}

static void FlashDrive_ResumeInterrupt(void)
{
    /* Resume all interrupts */
    if (0U == PriMaskReg)
    {
        __enable_irq();
    }
}
/*****************************************************************************
 * End file FlashDriver.c
 *****************************************************************************/