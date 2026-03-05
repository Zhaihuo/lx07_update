/**************************************************************************************************/
/**
 * @file      : main.c
 * @brief     : PreBoot - 最基础启动代码 + 跳转到 Bootloader
 * @version   : V1.0 (for PreBoot)
 * @date      : 2025-2026
 * @note      : 只做极简初始化 + 读取标志 + 跳转
 **************************************************************************************************/

#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"

#define BOOT_A_VALID (0xA5A5A5A5u)
#define BOOT_B_VALID (0xB5B5B5B5u)

/*256KB Flash 最后 8KB*/
#define METADATA_BASE_ADDR 0x0003E000u

// metadata 内偏移（可自定义结构体）
#define METADATA_MAGIC_OFFSET 0x00 // 魔数（可选校验）
#define ACTIVE_BOOT_OFFSET    0x04 // 当前活动 Boot：0 = Boot_A, 1 = Boot_B

/*boot_A和boot_B的起始地址*/
#define BOOT_A_START_ADDR 0x0000C000u // 0 + 48K（preboot size）
#define BOOT_B_START_ADDR 0x0001C000u // 0 + 48K（preboot size） + 64K（boot_A size）

/*魔数 用于校验meta数据是否有效*/
#define METADATA_MAGIC 0xC5C5C5C5u

static void PreBoot_Init(void)
{
    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();
    // 或者：WDOG_Feed();
    CLK_OSC40MEnable2(CLK_OSC_FREQ_MODE_HIGH, ENABLE, CLK_OSC_XTAL);
    CLK_SysClkSrc(CLK_SYS_FIRC64M);
    CLK_SetClkDivider(CLK_CORE, CLK_DIV_1);
    CLK_SetClkDivider(CLK_BUS, CLK_DIV_2);
    CLK_SetClkDivider(CLK_SLOW, CLK_DIV_8);
}

int main(void)
{
    PreBoot_Init();

    /*读取 metadata，跳转到Boot_A或者Boot_B*/
    volatile uint32_t *meta = (volatile uint32_t *)METADATA_BASE_ADDR;

    /*校验魔数（防止读到垃圾数据）*/
    if (meta[METADATA_MAGIC_OFFSET / 4] != METADATA_MAGIC)
    {
        // 魔数不对 → 默认跳转 Boot_A
        goto jump_to_boot_a;
    }

    uint32_t active_boot = meta[ACTIVE_BOOT_OFFSET / 4];

    uint32_t jump_addr;

    if (BOOT_A_VALID == active_boot)
    {
    jump_to_boot_a:
        jump_addr = BOOT_A_START_ADDR;
    }
    else if (BOOT_B_VALID == active_boot)
    {
        jump_addr = BOOT_B_START_ADDR;
    }
    else
    {
        while (1); // 非法数据，死循环
    }

    /*读取目标 Bootloader 的向量表*/
    uint32_t msp = *(volatile uint32_t *)jump_addr; // 栈顶指针

    uint32_t reset_handler = *(volatile uint32_t *)(jump_addr + 4); // 读取 Bootloader 的 Reset_Handler 地址

    /*安全检查*/
    if ((reset_handler & 0x01) == 0 || reset_handler < jump_addr)
    {
        while (1); // 非法地址，死循环
    }

    /*跳转前强制 VTOR 回默认（0x00000000），让 Bootloader 自己设置 VTOR*/
    SCB->VTOR = 0x00000000UL;

    /*清除所有 pending 中断（防止跳转后立即进错的中断）*/
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    /*关键：不要 __set_MSP()！保持 PreBoot 的 MSP*/
    /*Bootloader 的 Reset_Handler 会自己重新设置 MSP 和 VTOR*/

    ((void (*)(void))reset_handler)();

    while (1) {}
}