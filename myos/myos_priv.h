#ifndef __MYOS_PRIV_H
#define __MYOS_PRIV_H

#include <stm32f4xx_conf.h>
#include "myos_log.h"
#include "myos_cpu.h"

#define MYOS_MAX_TASKS  64
#define MYOS_STAT_RDY   0x00u
#define MYOS_STAT_SUS   0x01u
#define MYOS_STAT_IDLE  0x10u
#define MYOS_ERR_NONE           0u
#define MYOS_ERR_NO_MORE_TCB    1u
#define MYOS_FALSE  0u
#define MYOS_TRUE   1u

#define MYOS_TASK_IDLE_STK_SIZE 64u
#define MYOS_TASK_IDLE_STK_START (MYOS_TASK_IDLE_STK_SIZE - 1u)

#define MYOS_ENTER_CRITICAL() cpu_sr = myosCPUSaveSR()
#define MYOS_EXIT_CRITICAL()  myosCPURestoreSR(cpu_sr)
#define MYOS_TASK_SW()        myosCtxSw()

typedef struct _myos_TCB {
    myos_STK *myosTCBStkPtr;
    struct _myos_TCB *myosTCBNext;
    struct _myos_TCB *myosTCBPrev;
    INT16U myosTCBDly;
    INT16U myosTCBStat;
} myos_TCB;

myos_STK  myosTaskIdleStk[MYOS_TASK_IDLE_STK_SIZE];
myos_TCB  myosTCBTbl[MYOS_MAX_TASKS];
myos_TCB *myosTCBFreeList;
myos_TCB *myosTCBSusList;
myos_TCB *myosTCBListTail;      // the last task to run
myos_TCB *myosTCBListEntry;     // the next task to run
myos_TCB *myosTCBIdle;          // Idle task
myos_TCB *myosTCBCur;           // current running task
myos_TCB *myosTCBHighRdy;       // next task to run
INT8U myosRunning;
volatile INT32U myosTime;

#endif