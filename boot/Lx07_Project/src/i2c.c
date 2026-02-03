/**************************************************************************************************/
/**
 * @file     i2c.c
 * @brief
 * @version  V1.0.0
 * @date     December-2023
 * @author
 *
 * @note
 * Copyright (C) 2021-2023 Zhixin Semiconductor Ltd. All rights reserved.
 *
 **************************************************************************************************/

#include "i2c.h"
#include "Z20K11xM_gpio.h"
#include "Lx07.h"
#include "Eeprom.h"
#include "ProductLine.h"

// #define DISABLE_I2C0_ALL_INT

#define DES_REG_ADDR_BYTE 2

bool boI2c0IsIdle = true;

I2C_Config_t slaveConfig =
    {
        .masterSlaveMode = I2C_SLAVE,       /* I2C mode: slave */
        .speedMode       = I2C_SPEED_FAST,  /* Speed mode: fast mode 400kb/s */
        .addrBitMode     = I2C_ADDR_BITS_7, /* Address mode: 7 bits */
        .ownSlaveAddr    = 0x00000077,      /* Own slave address:0x00000077 */
        .restart         = DISABLE          /* Determines whether RESTART condition may be sent when acting as a master: enable */
};

I2C_Config_t masterConfig =
    {
        .masterSlaveMode = I2C_MASTER,         /* I2C mode: master */
        .speedMode       = I2C_SPEED_STANDARD, /* Speed mode: standard mode 100kb/s */
        .addrBitMode     = I2C_ADDR_BITS_7,    /* Address mode: 7 bits */
        .ownSlaveAddr    = 0x00000040,         /* Own slave address:0x00000090 */
        .restart         = ENABLE              /* Determines whether RESTART condition may be sent when acting as a master: enable */
};

I2C_Config_t masterConfig1 =
    {
        .masterSlaveMode = I2C_MASTER,         /* I2C mode: master */
        .speedMode       = I2C_SPEED_STANDARD, /* Speed mode: standard mode 100kb/s */
        .addrBitMode     = I2C_ADDR_BITS_7,    /* Address mode: 7 bits */
        .ownSlaveAddr    = 0x00000077,         /* Own slave address:0x00000090 */
        .restart         = ENABLE              /* Determines whether RESTART condition may be sent when acting as a master: enable */
};

const I2C_FifoConfig_t slaveFifoConfig =
    {
        .recvFifoThr     = 0, /* Receive  FIFO threshold level:3 */
        .transmitFifoThr = 2  /* Transmit FIFO threshold level:3 */
};

const I2C_FifoConfig_t masterFifoConfig =
    {
#ifdef DISABLE_I2C0_ALL_INT
        3,
        3
#else
        .recvFifoThr     = 0, /* Receive  FIFO threshold level:3 */
        .transmitFifoThr = 0  /* Transmit FIFO threshold level:3 */
#endif

};

void MstRecvByType(I2C_Id_t i2cNo, uint16_t len, I2C_RestartStop_t restartStopType)
{
    int delay_cnt = 0u;
    /* When TX FIFO is not full, the master sends a command to read */
    while (RESET == I2C_GetStatus(i2cNo, I2C_STATUS_TFNF))
    {
        if (delay_cnt > 5000) // test time
        {
            I2c_Init();
            delay_cnt = 0u;
            return;
        }
    }

    I2C_MasterReadCmd(i2cNo, restartStopType);
}

void MstSendByType(I2C_Id_t i2cNo, uint8_t *gTxBuffer, uint16_t len, I2C_RestartStop_t restartStopType)
{
    int delay_cnt = 0u;
    // for(int i = 0; i < len; i++)
    for (int i = (len - 1); i >= 0; i--)
    {
        /* When TX FIFO is not full, the master sends one byte */
        while (RESET == I2C_GetStatus(i2cNo, I2C_STATUS_TFNF))
        {
            delay_cnt++;
            if (delay_cnt > 5000) // test time
            {
                I2c_Init();
                delay_cnt = 0u;
                return;
            }
        }
        I2C_MasterSendByte(i2cNo, restartStopType, *(gTxBuffer + i));
    }
}

static void MstSendByType1(I2C_Id_t i2cNo, uint8_t *gTxBuffer, uint16_t len, I2C_RestartStop_t restartStopType)
{
    uint16_t delay_cnt = 0;
    for (int i = 0; i < len; i++)
    {
        /* When TX FIFO is not full, the master sends one byte */
        while (RESET == I2C_GetStatus(i2cNo, I2C_STATUS_TFNF))
        {
            delay_cnt++;
            if (delay_cnt > 5000) // test time
            {
                I2c_Init();
                delay_cnt = 0u;
                return;
            }
        }
        I2C_MasterSendByte(i2cNo, restartStopType, *(gTxBuffer + i));
    }
}

/* Write one byte of data to the destination address of the deserializer */
void Ex_MstWriteBuffer(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data)
{
    MstSendByType(i2cNo, DestAddr, DES_REG_ADDR_BYTE, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType(i2cNo, Data, 1, I2C_STOP_EN);
}

// lx07 往一个字节地址写入一个字节数据
void Lx07_MstWriteByte(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data)
{
    MstSendByType(i2cNo, DestAddr, 1, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType(i2cNo, Data, 1, I2C_STOP_EN);
}

void Ex_MstWriteBLKData(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data)
{
    MstSendByType(i2cNo, DestAddr, 1u, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType(i2cNo, Data, 2, I2C_STOP_EN);
}

void Ex_MstWriteBLK(I2C_Id_t i2cNo, uint8_t *Data)
{
    MstSendByType1(i2cNo, Data, 2u, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType1(i2cNo, Data + 2, 1, I2C_STOP_EN);
}

void Ex_MstWriteDoubleByte(I2C_Id_t i2cNo, uint8_t *Data)
{
    MstSendByType1(i2cNo, Data, 2u, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType1(i2cNo, Data + 2, 1, I2C_STOP_EN);
}

/* Write one byte of data to the destination address of the deserializer */
void Ex_MstWriteData(I2C_Id_t i2cNo, uint8_t *DestAddr, uint8_t *Data, uint8_t Len)
{
    MstSendByType(i2cNo, DestAddr, DES_REG_ADDR_BYTE, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType1(i2cNo, Data, Len - 1u, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType(i2cNo, Data, 1, I2C_STOP_EN);
}

void Ex_MstWriteArray(I2C_Id_t i2cNo, uint8_t *Data)
{
    MstSendByType1(i2cNo, Data, 7u, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType1(i2cNo, Data + 7, 1, I2C_STOP_EN);
}

void Ex_MstReadArray(I2C_Id_t i2cNo, uint8_t *DestAddr)
{
    MstSendByType(i2cNo, DestAddr, 1, I2C_RESTART_AND_STOP_DISABLE);
    MstRecvByType(i2cNo, 1, I2C_RESTART_AND_STOP_DISABLE);
}

/* Read one byte of data from the destination address of the deserializer */
void Ex_MstReadBuffer(I2C_Id_t i2cNo, uint8_t *DestAddr)
{
    MstSendByType(i2cNo, DestAddr, DES_REG_ADDR_BYTE, I2C_RESTART_AND_STOP_DISABLE);
    MstRecvByType(i2cNo, 1, I2C_STOP_EN);
}

void Ex_MstReadBuffersss(I2C_Id_t i2cNo, uint8_t *DestAddr)
{
    MstSendByType(i2cNo, DestAddr, 1, I2C_RESTART_AND_STOP_DISABLE);
    MstRecvByType(i2cNo, 1, I2C_STOP_EN);
}

void Ex_MstReadBLKData(I2C_Id_t i2cNo, uint8_t *DestAddr)
{
    MstSendByType(i2cNo, DestAddr, 1, I2C_RESTART_AND_STOP_DISABLE);
    MstRecvByType(i2cNo, 1, I2C_RESTART_AND_STOP_DISABLE);
}

void Ex_MstWriteEEP(I2C_Id_t i2cNo, uint8_t *Data, uint8_t length)
{
    MstSendByType1(i2cNo, Data, length, I2C_RESTART_AND_STOP_DISABLE);
    MstSendByType1(i2cNo, Data + length, 1, I2C_STOP_EN);
}

/*Lx07:Touch*/
void I2C_vSendBuffer(I2C_Id_t i2cId, uint8_t *txBuffer, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        /* When TX FIFO is not full, the master sends one byte */
        while (RESET == I2C_GetStatus(i2cId, I2C_STATUS_TFNF));
        I2C_MasterSendByte(i2cId, I2C_RESTART_AND_STOP_DISABLE, *(txBuffer + i));
    }
}

void I2C_vReadBuffer(I2C_Id_t i2cId, uint8_t *rxBuffer, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        /* When TX FIFO is not full, the master sends a command to read */
        while (RESET == I2C_GetStatus(i2cId, I2C_STATUS_TFNF));
        I2C_MasterReadCmd(i2cId, I2C_RESTART_AND_STOP_DISABLE);
        /* When RX FIFO is not empty, the master receives one byte */
        while (RESET == I2C_GetStatus(i2cId, I2C_STATUS_RFNE));
        *(rxBuffer + i) = I2C_ReceiveByte(i2cId);
    }
}

static void I2C_MasterRecvCallBack(void)
{
    I2C_ClearInt(I2C0_ID, I2C_INT_RX_FULL);
    boI2c0IsIdle = false;

    uint8_t u8tets = 0;

    ProductLine_vI2cRecvHandle();

    u8tets = I2C_ReceiveByte(I2C0_ID);
}

static void I2C1_MasterRecvCallBack(void)
{
    I2C_ClearInt(I2C1_ID, I2C_INT_RX_FULL);
}

static void I2C_MasterStopGeneratedCallBack(void)
{
    I2C_ClearInt(I2C0_ID, I2C_INT_STOP_DET);
    boI2c0IsIdle = true;

    ProductLine_vI2cStopHandle();
}

static void I2C1_MasterStopGeneratedCallBack(void)
{
    I2C_ClearInt(I2C1_ID, I2C_INT_STOP_DET);
}

void I2c_Init(void)
{
    /* I2C0 master mode config */
    CLK_ModuleSrc(CLK_I2C0, CLK_SRC_OSC40M);
    CLK_SetClkDivider(CLK_I2C0, CLK_DIV_2);
    SYSCTRL_ResetModule(SYSCTRL_I2C0);
    SYSCTRL_EnableModule(SYSCTRL_I2C0);

    PORT_PinmuxConfig(PORT_A, GPIO_2, PTA2_I2C0_SDA);
    PORT_PinmuxConfig(PORT_A, GPIO_3, PTA3_I2C0_SCL);

#ifdef DISABLE_I2C0_ALL_INT
    I2C_IntCmd(I2C0_ID, I2C_INT_ALL, DISABLE);
#else
    I2C_InstallCallBackFunc(I2C0_ID, I2C_INT_RX_FULL, I2C_MasterRecvCallBack);
    I2C_IntCmd(I2C0_ID, I2C_INT_RX_FULL, ENABLE);
    I2C_InstallCallBackFunc(I2C0_ID, I2C_INT_STOP_DET, I2C_MasterStopGeneratedCallBack);
    I2C_IntCmd(I2C0_ID, I2C_INT_STOP_DET, ENABLE);
#endif

    I2C_Disable(I2C0_ID);
    I2C_Init(I2C0_ID, &masterConfig);
    I2C_FIFOConfig(I2C0_ID, &masterFifoConfig);
    I2C_SetTargetAddr(I2C0_ID, 0x40);
    // I2C_SetTargetAddr(I2C0_ID, 0x48);
    I2C_Enable(I2C0_ID);
    NVIC_SetPriority(I2C0_IRQn, 0u);
    NVIC_EnableIRQ(I2C0_IRQn);

    /* I2C1 master mode config */
    CLK_ModuleSrc(CLK_I2C1, CLK_SRC_OSC40M);
    CLK_SetClkDivider(CLK_I2C1, CLK_DIV_2);
    SYSCTRL_ResetModule(SYSCTRL_I2C1);
    SYSCTRL_EnableModule(SYSCTRL_I2C1);

    PORT_PinmuxConfig(PORT_A, GPIO_0, PTA0_I2C1_SCL);
    PORT_PinmuxConfig(PORT_A, GPIO_1, PTA1_I2C1_SDA);

    I2C_InstallCallBackFunc(I2C1_ID, I2C_INT_RX_FULL, I2C1_MasterRecvCallBack);
    I2C_IntCmd(I2C1_ID, I2C_INT_RX_FULL, ENABLE);
    I2C_InstallCallBackFunc(I2C1_ID, I2C_INT_STOP_DET, I2C1_MasterStopGeneratedCallBack);
    I2C_IntCmd(I2C1_ID, I2C_INT_STOP_DET, ENABLE);

    I2C_Disable(I2C1_ID);
    I2C_Init(I2C1_ID, &masterConfig1);
    I2C_FIFOConfig(I2C1_ID, &masterFifoConfig);
    I2C_SetTargetAddr(I2C1_ID, 0x77u);
    I2C_Enable(I2C1_ID);
    NVIC_SetPriority(I2C1_IRQn, 0u);
    NVIC_EnableIRQ(I2C1_IRQn);
}
