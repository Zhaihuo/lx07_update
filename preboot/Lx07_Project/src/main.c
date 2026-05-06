/*****************************************************************************
 * @file main.c
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2026-04-24
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/

/*****************************************************************************
 * Include files
 *****************************************************************************/
#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"
/*****************************************************************************
 * Local macros
 *****************************************************************************/
#define BOOT_A_VALID      (0xA5A5A5A5)
#define BOOT_B_VALID      (0xB5B5B5B5)
#define BOOT_A_START_ADDR (0x0004000)
#define BOOT_B_START_ADDR (0x000E000)
#define BOOT_SIZE         (0x0000A000) // 40KB

#define SECTOR_SIZE (0x2000)

#define DFLASH_START (0x01000000)
#define DFLASH_SIZE  (0x00020000) /* 128KB */

#define DFLASH_BOOT_STATUS_ADDR (DFLASH_START + DFLASH_SIZE - SECTOR_SIZE)

/*Pflash总共256KB:0x00000000~0x0003FFFF,最后4KB剩余用作标志位读写区域*/
#define BOOT_VALID_STATUS_ADDR (0x0003F000)
/*****************************************************************************
 * Local data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/

/*****************************************************************************
 * Local function prototypes
 *****************************************************************************/
static void Preboot_vInit(void);
static void Preboot_vJumpToBoot(void);
/*****************************************************************************
 * function definitions
 *****************************************************************************/
int main(void)
{
    Preboot_vInit();
    Preboot_vJumpToBoot();
}

static void Preboot_vInit(void)
{
    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();
    CLK_OSC40MEnable2(CLK_OSC_FREQ_MODE_HIGH, ENABLE, CLK_OSC_XTAL);
    CLK_SysClkSrc(CLK_SYS_FIRC64M);
    CLK_SetClkDivider(CLK_CORE, CLK_DIV_1);
    CLK_SetClkDivider(CLK_BUS, CLK_DIV_2);
    CLK_SetClkDivider(CLK_SLOW, CLK_DIV_8);
}

static void Preboot_vJumpToBoot(void)
{
    void (*pBootResetHandler)(void);

    /*读取BOOT有效标志*/
    uint32_t u32BootStatus = *(volatile uint32_t *)DFLASH_BOOT_STATUS_ADDR;

    uint32_t u32BootStartAddr;
    if (BOOT_A_VALID == u32BootStatus)
    {
        u32BootStartAddr = BOOT_A_START_ADDR;
    }
    else if (BOOT_B_VALID == u32BootStatus)
    {
        u32BootStartAddr = BOOT_B_START_ADDR;
    }
    else
    {
        u32BootStartAddr = BOOT_A_START_ADDR;
    }

    pBootResetHandler = (void (*)(void))(*(volatile uint32_t *)(u32BootStartAddr + 4));

    pBootResetHandler();

    while (1) {}
}
/*****************************************************************************
 * End file main.c
 *****************************************************************************/
