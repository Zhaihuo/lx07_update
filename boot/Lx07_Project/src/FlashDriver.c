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
void FlashDrive_vInit(void)
{
    ResultStatus_t Sts = FlashDrive_BoardInit();

    guIntFlag = 0;
    /* Flash command complete interrupt */
    FLASH_InstallCallBackFunc(FLASH_INT_CCIF, FlashDrive_CCIFIntCallBack);
    /* enables flash interrupt in the NVIC interrupt controller */
    NVIC_EnableIRQ(FLASH_IRQn);
}

void FlashDrive_vHandler(void)
{
    uint32_t        u32Loop1 = 0;
    static uint32_t u32Loop2 = 0;

    static uint8_t u8Step = 0;

    volatile uint32_t *u32MetaData = (volatile uint32_t *)(0x0003E000);

    uint8_t au8EveryWriteData1[16] = {0xC5, 0xC5, 0xC5, 0xC5, 0xA5, 0xA5, 0xA5, 0xA5, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t au8EveryWriteData2[16] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66};

    if ((BOOT_A_VALID == u32MetaData[1]) || (BOOT_B_VALID == u32MetaData[1]))
    {
    }
    else
    {
        while (u8Step < 3)
        {
            switch (u8Step)
            {
                case 0:
                    if (FlashDrive_boEraseSector(0x0003E000))
                        u8Step++;
                    break;
                case 1:
                    if (FlashDrive_boProgramPhrase((0x0003E000 + u32Loop2), au8EveryWriteData1))

                    {
                        u32Loop2 += 16;
                        u8Step++;
                    }
                    if (FlashDrive_boProgramPhrase((0x0003E000 + u32Loop2), au8EveryWriteData1))
                        break;
                case 2:
                    if (FlashDrive_boProgramPhrase((0x0003E000 + u32Loop2), au8EveryWriteData2))
                    {
                        u32Loop2 += 16;
                        if (u32Loop2 >= SECTOR_SIZE)
                        {
                            u32Loop2 = SECTOR_SIZE;
                            u8Step++;
                        }
                    }
                    break;
                case 3:
                    /*Nothing*/
                    break;
                default:
                    /*Nothing*/
                    break;
            }
        }
    }
}

bool FlashDrive_boEraseSector(uint32_t u32Addr) // erase 0x2000 bytes (8k) every time
{
    static bool boIntFlg = true;

    if (SUCC == FLASH_EraseSector(u32Addr, &FlashDrive_CmdExeConfig))
    {
        /*擦除完成，恢复所有中断*/
        FlashDrive_ResumeInterrupt();
        boIntFlg = true;
        return true;
    }
    else
    {
        if (boIntFlg)
        {
            /*擦除前，关闭所有中断*/
            FlashDrive_SuspendInterrupt();
            boIntFlg = false;
        }

        return false;
    }
}

bool FlashDrive_boProgramPhrase(uint32_t u32Addr, uint8_t *pu8Data) // program 16 bytes every time
{
    static bool boIntFlg = true;

    if (SUCC == FLASH_ProgramPhrase(u32Addr, (uint8_t *)pu8Data, &FlashDrive_CmdExeConfig))
    {
        /*擦除完成，恢复所有中断*/
        FlashDrive_ResumeInterrupt();
        boIntFlg = true;
        return true;
    }
    else
    {
        if (boIntFlg)
        {
            /*擦除前，关闭所有中断*/
            FlashDrive_SuspendInterrupt();
            boIntFlg = false;
        }

        return false;
    }
}

// bool FlashDrive_boProgramData(uint32_t u32Addr, uint8_t *pu8Data, uint16_t u16Len) // u16Len可选：128，256，512，1024
// {
// }

static ResultStatus_t FlashDrive_BoardInit(void)
{
    /* Interrupt vector table redefinition*/
    SCB->VTOR = ((uint32_t)&__vector_table) & 0xFFFFFF00U;
    __DSB();

    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();

    return SUCC;
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