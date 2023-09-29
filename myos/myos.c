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

myos_STK *myos_TaskStkInit(  void (*task)(void *p_arg),
                            void *p_arg,
                            myos_STK *p_stk) {
    *(p_stk) = (myos_STK *)task;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)0;
    *(--p_stk) = (INT32U)p_arg;
    *(--p_stk) = (INT32U)0x01000000L;
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
    err = myos_TCBInit(p_stk);
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
    myosTCBFreeList = &myosTCBTbl[0];
}

void  myos_TaskIdle (void *p_arg)
{
    (void)p_arg;                                 /* Prevent compiler warning for not using 'p_arg'     */
    for (;;) {
    }
}

void myos_InitTaskIdle(void) {
    (void)myos_TaskCreate(myos_TaskIdle, (void *)0, &myosTaskIdleStk[MYOS_TASK_IDLE_STK_SIZE - 1u]);
}

void myos_Init(void) {
    myosRunning = MYOS_FALSE;
    myosTCBCur = (myos_TCB *)0u;
    myosTCBHighRdy = (myos_TCB *)0u;
    myos_InitTCBList();
    myos_InitTaskIdle();
}

void myos_Start(void) {
    if(myosRunning == MYOS_FALSE) {
        myos_SchedNew();
        myosStartHighRdy();
    }
}