/*****************************************************************************
 * @file Config.h
 *
 * @author
 *
 * @version 1.0
 *
 * @date 2026-04-17
 *
 * @copyright Wuhan Baohua Display Technology Co., Ltd.
 *****************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

/*****************************************************************************
 * Include files
 *****************************************************************************/

/*****************************************************************************
 * Global macros
 *****************************************************************************/
#define BOOT_A_START_ADDR (0x00004000)
#define BOOT_B_START_ADDR (0x0000E000)
#define BOOT_SIZE         (0x0000A000) /* 40KB */

#define APP_A_START_ADDR (0x00018000)
#define APP_B_START_ADDR (0x0002B800)
#define APP_SIZE         (0x00013800) /* 78KB */

#define SECTOR_SIZE (0x2000)

#define DFLASH_START (0x01000000)
#define DFLASH_SIZE  (0x00020000) /* 128KB */

#define DFLASH_BOOT_STATUS_ADDR (DFLASH_START + DFLASH_SIZE - SECTOR_SIZE)

#define REGFILE_ID_00           (0)
#define REGFILE_ID_01           (1)
#define REGFILE_UPDATE_BOOT_FLG (0x01010101)
#define REGFILE_UPDATE_APP_FLG  (0x02020202)

#define BOOT_A_VALID (0xA5A5A5A5)
#define BOOT_B_VALID (0xB5B5B5B5)

/*Pflash总共256KB:0x00000000~0x0003FFFF,最后4KB剩余用作标志位读写区域*/
#define BOOT_VALID_STATUS_ADDR (0x0003F000)

#define ELSE_NOTHING \
    else {}
/*****************************************************************************
 * Global data types
 *****************************************************************************/

/*****************************************************************************
 * Variant declarations
 *****************************************************************************/

/*****************************************************************************
 * Global function prototypes
 *****************************************************************************/

#endif
/*****************************************************************************
 * End file CONFIG_H
 *****************************************************************************/
