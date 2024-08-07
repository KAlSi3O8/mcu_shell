#include "myos_priv.h"

void myos_MemClr(INT8U *ptr, INT16U size) {
    while(size > 0u) {
        *ptr = 0u;
        ptr++;
        size--;
    }
}

INT8U myos_TCBInit(myos_STK *p_stk, INT16U TCB_stat) {
    myos_TCB *ptcb;
    myos_CPU_SR cpu_sr = 0;

    MYOS_ENTER_CRITICAL();
    ptcb = myosTCBFreeList;
    if(ptcb != 0u) {
        myosTCBFreeList = ptcb->myosTCBNext;
        ptcb->myosTCBStkPtr = p_stk;
        ptcb->myosTCBDly = 0u;
        ptcb->myosTCBStat = TCB_stat;

        if(TCB_stat == MYOS_STAT_IDLE) {
            ptcb->myosTCBPrev = 0u;
            ptcb->myosTCBNext = 0u;
            myosTCBIdle = ptcb;
        } else {
            ptcb->myosTCBPrev = myosTCBListTail;
            ptcb->myosTCBNext = 0u;
            if(myosTCBListTail != 0u) {
                myosTCBListTail->myosTCBNext = ptcb;
            }
            if(myosTCBListEntry == 0u) {
                myosTCBListEntry = ptcb;
            }
            myosTCBListTail = ptcb;
        }
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
    // save last TCB
    if(myosTCBCur != 0u) {
        if(myosTCBCur->myosTCBStat == MYOS_STAT_RDY) {
            myosTCBCur->myosTCBPrev = myosTCBListTail;
            if(myosTCBListTail != 0u) {
                myosTCBListTail->myosTCBNext = myosTCBCur;
            }
            myosTCBListTail = myosTCBCur;
            if(myosTCBListEntry == 0) {
                myosTCBListEntry = myosTCBCur;
            }
        }
        else if(myosTCBCur->myosTCBStat == MYOS_STAT_SUS) {
            myosTCBCur->myosTCBNext = myosTCBSusList;
            myosTCBCur->myosTCBPrev = 0u;
            if(myosTCBSusList != 0) {
                myosTCBSusList->myosTCBPrev = myosTCBCur;
            }
            myosTCBSusList = myosTCBCur;
        }
    }

    // get next TCB
    if(myosTCBListEntry != 0) {
        myosTCBHighRdy = myosTCBListEntry;
        myosTCBListEntry = myosTCBHighRdy->myosTCBNext;
        if(myosTCBListEntry != 0u) {
            myosTCBListEntry->myosTCBPrev = 0u;
        } else {
            // the head is empty so the tail must be empty
            myosTCBListTail = 0u;
        }
        myosTCBHighRdy->myosTCBNext = 0u;
        myosTCBHighRdy->myosTCBPrev = 0u;
    } else {
        myosTCBHighRdy = myosTCBIdle;
    }
}

void myos_Sched(void) {
    myos_CPU_SR cpu_sr = 0;

    MYOS_ENTER_CRITICAL();
    myos_SchedNew();
    SysTick->CTRL;
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
    err = myos_TCBInit(psp, MYOS_STAT_RDY);
    if(err == MYOS_ERR_NONE) {
        if(myosRunning == MYOS_TRUE) {
            myos_Sched();
        }
    }
    return err;
}

INT8U myos_IdleTaskCreate( void (*task)(void *p_arg),
                        void *p_arg,
                        myos_STK *p_stk) {
    myos_STK *psp;
    INT8U err;

    psp = myos_TaskStkInit(task, p_arg, p_stk);
    err = myos_TCBInit(psp, MYOS_STAT_IDLE);
    return err;
}

void myos_InitTCBList(void) {
    INT8U i;

    myos_MemClr((INT8U * )&myosTCBTbl[0], sizeof(myosTCBTbl));

    for(i = 0u; i < MYOS_MAX_TASKS - 1u; i++) {
        myosTCBTbl[i].myosTCBNext = &myosTCBTbl[i + 1u];
    }
    myosTCBTbl[i].myosTCBNext = 0u;

    myosTCBListTail = 0u;
    myosTCBListEntry = 0u;
    myosTCBSusList = 0u;
    myosTCBFreeList = &myosTCBTbl[0];
}

void myos_TaskIdle (void *p_arg) {
    (void)p_arg;                                 /* Prevent compiler warning for not using 'p_arg'     */
    for (;;) {
    }
}

void myos_InitTaskIdle(void) {
    NVIC_SetPriority(PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    (void)myos_IdleTaskCreate(myos_TaskIdle, (void *)0, &myosTaskIdleStk[MYOS_TASK_IDLE_STK_START]);
}

void myos_InitSystick(void) {
    SysTick_Config(168000);
}

void myos_Init(void) {
    myosRunning = MYOS_FALSE;
    myosTime = 0u;
    myosTCBIdle = (myos_TCB *)0u;
    myosTCBCur = (myos_TCB *)0u;
    myosTCBHighRdy = (myos_TCB *)0u;
    myos_InitTCBList();
    myos_InitTaskIdle();
}

void myos_Start(void) {
    if(myosRunning == MYOS_FALSE) {
        myos_SchedNew();
        myos_InitSystick();
        myosStartHighRdy();
    }
}

void myos_TimeDly(INT16U ticks) {
    myos_CPU_SR cpu_sr = 0u;

    if(ticks > 0u) {
        MYOS_ENTER_CRITICAL();
        myosTCBCur->myosTCBDly = ticks;
        myosTCBCur->myosTCBStat = MYOS_STAT_SUS;
        MYOS_EXIT_CRITICAL();
        myos_Sched();
    }
}

void SysTick_Handler(void) {
    myos_TCB *ptcb;
    myos_TCB *ptcb_prev;
    myos_TCB *ptcb_next;

    if(myosRunning == 1) {
        myosTime++;
        ptcb = myosTCBSusList;
        while(ptcb != 0u) {
            ptcb->myosTCBDly--;
            if(ptcb->myosTCBDly == 0u) {
                // take tcb away from suspend list
                ptcb_prev = ptcb->myosTCBPrev;
                ptcb_next = ptcb->myosTCBNext;
                if(ptcb_prev != 0u) {
                    ptcb_prev->myosTCBNext = ptcb_next;
                } else {
                    // the head of suspend list leave, move the head to the next one
                    myosTCBSusList = ptcb_next;
                }
                if(ptcb_next != 0u) {
                    ptcb_next->myosTCBPrev = ptcb_prev;
                }
                // return tcb to ready list
                ptcb->myosTCBStat = MYOS_STAT_RDY;
                ptcb->myosTCBPrev = myosTCBListTail;
                ptcb->myosTCBNext = 0u;
                if(myosTCBListTail != 0u) {
                    myosTCBListTail->myosTCBNext = ptcb;
                }
                myosTCBListTail = ptcb;
                if(myosTCBListEntry == 0) {
                    myosTCBListEntry = ptcb;
                }
                // next suspend task
                ptcb = ptcb_next;
                continue;
            }
            ptcb = ptcb->myosTCBNext;
        }
        myos_Sched();
        MYOS_TASK_SW();
    }
}