/**************************************************************************************************/
/**
 * @file      : main.c
 * @brief     : PreBoot - 最基础启动代码 + 跳转到 Bootloader
 * @version   : V1.0 (for PreBoot)
 * @date      : 2025-2026
 * @note      : 只做极简初始化 + 读取标志 + 跳转，不初始化复杂外设
 **************************************************************************************************/

#include "Z20K11xM_drv.h"
#include "Z20K11xM_clock.h"
#include "Z20K11xM_sysctrl.h"
#include "Z20K11xM_wdog.h"

// ──────────────────────────────────────────────
//  PreBoot 配置区（根据你的实际分区调整这些宏）
// ──────────────────────────────────────────────

// Metadata 存放地址（建议放在 Flash 最后 1~4KB，避免被 Boot/App 覆盖）
#define METADATA_BASE_ADDR      0x0003F000U     // 示例：256KB Flash 最后 4KB

// metadata 内偏移（可自定义结构体）
#define METADATA_MAGIC_OFFSET   0x00            // 魔数（可选校验）
#define ACTIVE_BOOT_OFFSET      0x04            // 当前活动 Boot：0 = Boot_A, 1 = Boot_B

// Bootloader 起始地址（与 scatter 和分区规划一致）
#define BOOT_A_START_ADDR       0x0000C000U     // PreBoot 48KB 后
#define BOOT_B_START_ADDR       0x0001C000U     // Boot_A 64KB 后

// 魔数（可选，用于校验 metadata 是否有效）
#define METADATA_MAGIC          0x5A5AA5A5U

/* delay function (仅用于测试，正式可删除) */
static void delay(volatile uint32_t cycles)
{
    while (cycles--);
}

static void PreBoot_Init(void)
{
    // 1. 关闭或喂狗（防止 PreBoot 执行期间复位）
    SYSCTRL_EnableModule(SYSCTRL_WDOG);
    WDOG_Disable();                     // 推荐：直接禁用
    // 或者：WDOG_Feed();               // 如果必须启用看门狗

    // 2. 时钟配置（最简配置，确保后续能跑）
    //    注意：PreBoot 不建议开太多时钟源，保持简单
    CLK_OSC40MEnable2(CLK_OSC_FREQ_MODE_HIGH, ENABLE, CLK_OSC_XTAL);
    CLK_SysClkSrc(CLK_SYS_FIRC64M);     // 或 CLK_SYS_OSC40M，根据你的需求
    CLK_SetClkDivider(CLK_CORE, CLK_DIV_1);
    CLK_SetClkDivider(CLK_BUS,  CLK_DIV_2);
    CLK_SetClkDivider(CLK_SLOW, CLK_DIV_8);
}

int main(void)
{
    PreBoot_Init();

    // ──────────────────────────────────────────────
    //  读取 metadata，决定跳转到哪个 Bootloader
    // ──────────────────────────────────────────────

    volatile uint32_t *meta = (volatile uint32_t *)METADATA_BASE_ADDR;

    // 可选：校验魔数（防止读到垃圾数据）
    if (meta[METADATA_MAGIC_OFFSET / 4] != METADATA_MAGIC)
    {
        // 魔数不对 → 默认跳转 Boot_A（或进入死循环/安全模式）
        goto jump_to_boot_a;
    }

    uint32_t active_boot = meta[ACTIVE_BOOT_OFFSET / 4];  // 0 = A, 1 = B, 其他值默认 A

    uint32_t jump_addr;
    if (active_boot == 1)
    {
        jump_addr = BOOT_B_START_ADDR;
    }
    else
    {
jump_to_boot_a:
        jump_addr = BOOT_A_START_ADDR;
    }

    // ──────────────────────────────────────────────
    //  标准 Cortex-M 跳转到另一个 image 的方式
    // ──────────────────────────────────────────────

    // 读取目标 Bootloader 的向量表
    uint32_t msp           = *(volatile uint32_t *)jump_addr;         // 栈顶指针
    uint32_t reset_handler = *(volatile uint32_t *)(jump_addr + 4);   // Reset_Handler 地址

    // 设置主栈指针（MSP）
    __set_MSP(msp);

    // 清除所有 pending 中断（可选，但推荐）
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    // 跳转执行（直接调用函数指针）
    ((void (*)(void))reset_handler)();

    while (1) {}
}