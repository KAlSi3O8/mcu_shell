#ifndef __MYDRIVER_IIC_H
#define __MYDRIVER_IIC_H

#include <stdint.h>

void IIC_Init(void);
void IIC_DMAWriteBytes(uint8_t addr, uint8_t *byte, uint32_t size);
void IIC_WriteBytes(uint8_t addr, uint8_t ctl, uint8_t *byte, uint32_t size);

#endif