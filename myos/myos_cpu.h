#ifndef __MYOS_CPU_H
#define __MYOS_CPU_H

typedef char INT8;
typedef unsigned char INT8U;

typedef short INT16;
typedef unsigned short INT16U;

typedef int INT32;
typedef unsigned int INT32U;

typedef INT32U myos_STK;
typedef INT32U myos_CPU_SR;

myos_CPU_SR myosCPUSaveSR(void);
void myosCPURestoreSR(myos_CPU_SR cpu_sr);
void myosStartHighRdy(void);
void myosCtxSw(void);

#endif