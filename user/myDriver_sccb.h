#ifndef __MYDRIVER_SCCB_H
#define __MYDRIVER_SCCB_H

#include <stdint.h>

void SCCB_Init(void);
uint8_t SCCB_ReadReg(uint8_t addr, uint8_t reg);

#endif