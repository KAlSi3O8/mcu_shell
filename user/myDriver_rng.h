#ifndef __MYDRIVER_RNG_H
#define __MYDRIVER_RNG_H

#include <stdint.h>

void RNG_Init(void);
uint32_t RNG_RandomIn(uint32_t range);

#endif