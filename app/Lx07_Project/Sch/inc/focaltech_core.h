/*
 * Privileged & confidential  All Rights/Copyright Reserved by FocalTech.
 *       ** Source code released bellows and hereby must be retained as
 * FocalTech's copyright and with the following disclaimer accepted by
 * Receiver.
 *
 * "THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE FOCALTECH'S AND ITS AFFILIATES'DIRECTORS AND OFFICERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
 */

#ifndef __FOCALTECH_CORE_H__
#define __FOCALTECH_CORE_H__
/*****************************************************************************
 * Included header files
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
// #include <unistd.h>
#include "Z20k11xM_i2c.h"
#include "Uart2.h"
/*****************************************************************************
 * Private constant and macro definitions using #define
 *****************************************************************************/
#define HOST_MCU_DRIVER_VERSION "FocalTech MCU V1.2 20250926"

#define FTS_INFO(fmt, ...)  Uart2_vprintf("[FTS/I]%s:" fmt "\n", __func__, ##__VA_ARGS__)
#define FTS_ERROR(fmt, ...) Uart2_vprintf("[FTS/E]%s:" fmt "\n", __func__, ##__VA_ARGS__)
#define FTS_DEBUG(fmt, ...) Uart2_vprintf("[FTS/D]%s:" fmt "\n", __func__, ##__VA_ARGS__)

/* chip id */
#define FTS_CHIP_IDH 0x72
#define FTS_CHIP_IDL 0x54
/* rom id */
#define FTS_ROM_IDH 0x72
#define FTS_ROM_IDL 0x54
/* pramboot id */
#define FTS_PRAM_IDH 0x72
#define FTS_PRAM_IDL 0xA4

/*****************************************************************************
 * Private enumerations, structures and unions using typedef
 *****************************************************************************/
/*
 * Structures of point information
 *
 * @x: X coordinate of this point
 * @y: Y coordinate of this point
 * @p: pressure value of this point
 * @type: event type of this point, 0 means down event,
 *        1 means up event, 2 means contact event
 * @id: ID of this point
 * @area: touch area of this point
 */
struct fts_ts_event
{
    int x;    /*x coordinate */
    int y;    /*y coordinate */
    int p;    /* pressure */
    int type; /* touch event flag: 0 -- down; 1-- up; 2 -- contact */
    int id;   /*touch ID */
    int area; /*touch area*/
};

/*****************************************************************************
 * Global variable or extern global variabls/functions
 *****************************************************************************/

void fts_msleep(unsigned long msec);

/* communication interface */
int fts_read(uint8_t addr, uint8_t *data, uint16_t datalen);
int fts_read_reg(uint8_t addr, uint8_t *value);
int fts_write(uint8_t addr, uint8_t *data, uint16_t datalen);
int fts_write_reg(uint8_t addr, uint8_t value);
int fts_write_read(uint8_t *addr, uint16_t addrlen, uint8_t *data, uint16_t datalen);

int fts_host_test(void);

int fts_ts_init(void);
int fts_hw_reset(uint32_t delay_ms);
int fts_ft7254_write_backup_gamma(uint8_t *buf, uint32_t len, uint8_t mode);

int platform_i2c_write(uint8_t *txBuffer, uint16_t len);
int platform_i2c_read(uint8_t *txBuffer, uint16_t len);

extern void I2C_vSendBuffer(I2C_Id_t i2cId, uint8_t *txBuffer, uint16_t len);
extern void I2C_vReadBuffer(I2C_Id_t i2cId, uint8_t *rxBuffer, uint16_t len);

int fts_gpio_interrupt_handler(void);
#endif /* __FOCALTECH_CORE_H__ */
