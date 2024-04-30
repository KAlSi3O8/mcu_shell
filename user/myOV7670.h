#ifndef __MYOV7670_H
#define __MYOV7670_H

#include <stdint.h>

#define OV7670_ADDR         0x42
#define OV7670_WIDTH        64
#define OV7670_HEIGHT       48
#define OV7670_RATE         8
#define OV7670_BUF_SIZE     (OV7670_WIDTH * OV7670_HEIGHT)

struct s_buf {
    uint8_t Layer1;
    uint8_t Layer0;
};

extern uint8_t grayThreshold;
extern struct s_buf OV7670_Buf[OV7670_HEIGHT][OV7670_WIDTH];

uint16_t OV7670_GetMID(void);
uint16_t OV7670_GetPID(void);
uint16_t OV7670_GetAEC(void);
int OV7670_SoftReset(void);
int OV7670_Init(void);

#endif