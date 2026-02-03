/**************************************************************************************************/
/**
 * @file     i2c.h
 * @brief
 * @version  V1.0.0
 * @date     December-2023
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/

#ifndef I2C_H
#define I2C_H

#include "Z20K11xM_drv.h"
#include "Z20K11xM_i2c.h"
#include "serial.h"
#include "string.h"
#include "stdbool.h"

extern void I2c_Init(void);

extern bool boI2c0IsIdle;

/* master */
extern void MstRecvByType(I2C_Id_t i2cNo, uint16_t len, I2C_RestartStop_t restartStopType);
extern void MstSendByType(I2C_Id_t i2cNo, uint8_t *gTxBuffer, uint16_t len, I2C_RestartStop_t restartStopType);
extern void Ex_MstWriteBuffer(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data);
void        Lx07_MstWriteByte(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data);
extern void Ex_MstWriteArray(I2C_Id_t i2cNo, uint8_t *Data);
extern void Ex_MstReadArray(I2C_Id_t i2cNo, uint8_t *DestAddr);
extern void Ex_MstReadBuffer(I2C_Id_t i2cNo, uint8_t *DestAddr);
extern void Ex_MstWriteData(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data, uint8_t Len);
extern void Ex_MstWriteBLKData(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data);
extern void Ex_MstReadBLKData(I2C_Id_t i2cNo, uint8_t *DestAddr);
extern void Ex_MstWriteBLK(I2C_Id_t i2cNo, uint8_t *Data);
extern void Ex_MstWriteDoubleByte(I2C_Id_t i2cNo, uint8_t *Data);
extern void Ex_MstReadBuffersss(I2C_Id_t i2cNo, uint8_t *Data);
extern void Ex_MstWriteEEP(I2C_Id_t i2cNo, uint8_t *Data, uint8_t length);

/* callback */
static void I2C_MasterRecvCallBack(void);
static void I2C_MasterStopGeneratedCallBack(void);

#endif /* I2C_H */
