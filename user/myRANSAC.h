#ifndef __MYRANSAC_H
#define __MYRANSAC_H

#include <stdint.h>

#define POS_LIST_SIZE (64 * 48)

struct s_XY {
    uint8_t x;
    uint8_t y;
};
struct s_pos {
    uint16_t index;
    struct s_XY pos[POS_LIST_SIZE];
};
struct s_cir_out {
    uint16_t cnt;
    uint8_t x;
    uint8_t y;
};
struct s_arc_out {
    uint16_t cnt;
    struct s_XY max;
    struct s_XY min;
};

extern struct s_pos OV7670_List;

void RANSAC(void);
void GaussianFilter(void);
void EdgeDetection(void);

#endif