#ifndef __MYOS_H
#define __MYOS_H

#include "myos_cpu.h"

void myos_Init(void);
void myos_Start(void);
INT8U myos_TaskCreate( void (*task)(void *p_arg), void *p_arg, myos_STK *p_stk);

#endif