#ifndef __MYOV7670_H
#define __MYOV7670_H

#include <stdint.h>

#define OV7670_ADDR         0x42

extern uint8_t grayThreshold;

uint16_t OV7670_GetMID(void);
uint16_t OV7670_GetPID(void);
uint16_t OV7670_GetAEC(void);
int OV7670_SoftReset(void);
int OV7670_Init(void);

#endif