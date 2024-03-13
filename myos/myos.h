#ifndef __MYOS_H
#define __MYOS_H

#include "myos_cpu.h"

#define MYOS_TASK_STK_SIZE 64u
#define MYOS_TASK_STK_START (MYOS_TASK_STK_SIZE - 1u)

void myos_Init(void);
void myos_Start(void);
INT8U myos_TaskCreate( void (*task)(void *p_arg), void *p_arg, myos_STK *p_stk);
void myos_TimeDly(INT32U ticks);

#endif