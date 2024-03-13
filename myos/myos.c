#include "myos_priv.h"

void myos_MemClr(INT8U *ptr, INT16U size) {
    while(size > 0u) {
        *ptr = 0u;
        ptr++;
        size--;
    }
}

INT8U myos_TCBInit(myos_STK *p_stk) {
    myos_TCB *ptcb;
    myos_CPU_SR cpu_sr = 0;

    MYOS_ENTER_CRITICAL();
    ptcb = myosTCBFreeList;
    if(ptcb != 0u) {
        myosTCBFreeList = ptcb->myosTCBNext;
        ptcb->myosTCBStkPtr = p_stk;
        ptcb->myosTCBDly = 0u;
        ptcb->myosTCBStat = MYOS_STAT_RDY;

        ptcb->myosTCBPrev = myosTCBList;
        ptcb->myosTCBNext = 0u;
        if(myosTCBList != 0u) {
            myosTCBList->myosTCBNext = ptcb;
        }
        if(myosTCBListEntry == 0u) {
            myosTCBListEntry = ptcb;
        }
        myosTCBList = ptcb;
        MYOS_EXIT_CRITICAL();
        return MYOS_ERR_NONE;
    }
    MYOS_EXIT_CRITICAL();
    return MYOS_ERR_NO_MORE_TCB;
}

/* 
 * When pushing context to the stack, the hardware saves eight 32-bit words, 
 * comprising xPSR, ReturnAddress, LR(R14), R12, R3, R2, R1, and R0.
 */
myos_STK *myos_TaskStkInit(  void (*task)(void *p_arg),
                            void *p_arg,
                            myos_STK *p_stk) {
    *(p_stk) = (INT32U)0x01000000L;     // PSR
    *(--p_stk) = (INT32U)task;          // PC
    *(--p_stk) = (INT32U)0;             // LR
    *(--p_stk) = (INT32U)0;             // R12
    *(--p_stk) = (INT32U)0;             // R3
    *(--p_stk) = (INT32U)0;             // R2
    *(--p_stk) = (INT32U)0;             // R1
    *(--p_stk) = (INT32U)p_arg;         // R0
    *(--p_stk) = (INT32U)0;             // R11
    *(--p_stk) = (INT32U)0;             // R10
    *(--p_stk) = (INT32U)0;             // R9
    *(--p_stk) = (INT32U)0;             // R8
    *(--p_stk) = (INT32U)0;             // R7
    *(--p_stk) = (INT32U)0;             // R6
    *(--p_stk) = (INT32U)0;             // R5
    *(--p_stk) = (INT32U)0;             // R4
    return p_stk;
}

void myos_SchedNew(void) {
//save last TCB
    if(myosTCBList != myosTCBCur) {
        myosTCBList->myosTCBNext = myosTCBCur;
        myosTCBCur->myosTCBPrev = myosTCBList;
        myosTCBList = myosTCBCur;
    }

//get next TCB
    myosTCBHighRdy = myosTCBListEntry;
    if(myosTCBHighRdy->myosTCBNext != 0) {
        myosTCBListEntry = myosTCBHighRdy->myosTCBNext;
        myosTCBListEntry->myosTCBPrev = 0u;
        myosTCBHighRdy->myosTCBNext = 0u;
    }
}

void myos_Sched(void) {
    myos_CPU_SR cpu_sr = 0;

    MYOS_ENTER_CRITICAL();
    myos_SchedNew();
    if(myosTCBHighRdy != myosTCBCur) {
        MYOS_TASK_SW();
    }
    MYOS_EXIT_CRITICAL();
}

INT8U myos_TaskCreate( void (*task)(void *p_arg),
                        void *p_arg,
                        myos_STK *p_stk) {
    myos_STK *psp;
    INT8U err;

    psp = myos_TaskStkInit(task, p_arg, p_stk);
    err = myos_TCBInit(psp);
    if(err == MYOS_ERR_NONE) {
        if(myosRunning == MYOS_TRUE) {
            myos_Sched();
        }
    }
    return err;
}

void myos_InitTCBList(void) {
    INT8U i;

    myos_MemClr((INT8U *)&myosTCBTbl[0], sizeof(myosTCBTbl));

    for(i = 0u; i < MYOS_MAX_TASKS - 1u; i++) {
        myosTCBTbl[i].myosTCBNext = &myosTCBTbl[i + 1u];
    }
    myosTCBTbl[i].myosTCBNext = 0u;

    myosTCBList = 0u;
    myosTCBSusList = 0u;
    myosTCBFreeList = &myosTCBTbl[0];
}

void myos_TaskIdle (void *p_arg) {
    (void)p_arg;                                 /* Prevent compiler warning for not using 'p_arg'     */
    myos_log("myos_TaskIdle Entry-->");
    for (;;) {
    }
}

void myos_InitTaskIdle(void) {
    NVIC_SetPriority(PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    (void)myos_TaskCreate(myos_TaskIdle, (void *)0, &myosTaskIdleStk[MYOS_TASK_IDLE_STK_START]);
}

void myos_InitSystick(void) {
    SysTick_Config(168000);
}

void myos_Init(void) {
    myosRunning = MYOS_FALSE;
    myosTime = 0u;
    myosTCBCur = (myos_TCB *)0u;
    myosTCBHighRdy = (myos_TCB *)0u;
    myos_InitTCBList();
    myos_InitTaskIdle();
}

void myos_Start(void) {
    if(myosRunning == MYOS_FALSE) {
        myos_SchedNew();
        // myos_InitSystick();
        myosStartHighRdy();
    }
}

void myos_TimeDly(INT16U ticks) {
    myos_CPU_SR cpu_sr = 0u;

    if(ticks > 0u) {
        MYOS_ENTER_CRITICAL();
        if(myosTCBSusList != 0) {
            myosTCBCur->myosTCBNext = myosTCBSusList;
            myosTCBCur->myosTCBPrev = 0u;
            myosTCBSusList->myosTCBPrev = myosTCBCur;
        }
        myosTCBCur->myosTCBDly = ticks;
        MYOS_EXIT_CRITICAL();
        myos_Sched();
    }
}

void SysTick_Handler(void) {
    myos_TCB *ptcb;
    myos_TCB *ptcbh;
    myos_CPU_SR cpu_sr = 0;

    myos_log("SysTick Handler Entry-->");
    if(myosRunning == 1) {
        MYOS_ENTER_CRITICAL();
        myosTime++;
        ptcb = myosTCBSusList;
        while(ptcb != 0u) {
            ptcb->myosTCBDly--;
            if(ptcb->myosTCBDly == 0u) {
                ptcbh = ptcb->myosTCBPrev;
                ptcb->myosTCBStat &= ~MYOS_STAT_SUS;
                ptcbh->myosTCBNext = ptcb->myosTCBNext;
                ptcb->myosTCBNext->myosTCBPrev = ptcbh;
                ptcb->myosTCBPrev = myosTCBList;
                ptcb->myosTCBNext = 0u;
                if(myosTCBList != 0u) {
                    myosTCBList->myosTCBNext = ptcb;
                }
                myosTCBList = ptcb;
                ptcb = ptcbh;
            }
            ptcb = ptcb->myosTCBNext;
        }
        myos_Sched();
        myosCtxSw();
        MYOS_EXIT_CRITICAL();
    }
}