#ifndef __MYOV7670_H
#define __MYOV7670_H

#include <stdint.h>

uint16_t OV7670_GetMID(void);
uint16_t OV7670_GetPID(void);
int OV7670_SoftReset(void);
int OV7670_Init(void);

#endif