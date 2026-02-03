#ifndef _SCH_CFG_H_
#define _SCH_CFG_H_

#define SCH_PERIODIC_MAX_NUM  6
#define SCH_1MSCOUNTER(Clock) (Clock * 1000)
#define SCH_EndOfList         (Sch_Task_T)0
#define SCH_MS(Ms)            Ms
#define SCH_TIMEDURATION(Idx) (SCH_1MSCOUNTER(64) * (Idx))

typedef struct TaskTablePara
{
    uint8_t  u8CurExecuteIdx;
    uint8_t  u8TaskStartPosition;
    uint8_t  u8TaskEndPosition;
    uint32_t u32TimeDuration;
} TaskTablePara;

extern void Sch_Main(void);
extern void GPIOIntInit(void);

extern const Sch_Task_Info_T Sch_RunTaskTable[];
extern uint8_t               u8BatCmdFlag;
extern uint8_t               u8RegStsCmdFlag;
extern uint16_t              u16BatVoltageVal;
extern uint8_t               MstRecvBuf[64];
extern uint8_t               u8StartReadTouch;

#endif
