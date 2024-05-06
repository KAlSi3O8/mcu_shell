#include "myDriver_rng.h"
#include "mySSD1306.h"
#include "myOV7670.h"
#include "myRANSAC.h"
#include <stdio.h>

struct s_pos OV7670_List;

void RANSAC(void) {
    int32_t x0, y0, r0;
    int32_t x1, y1, r1;
    int32_t x2, y2, r2;
    int32_t dx, dy, dr;
    float   a, b, m1, m2;
    uint32_t rand, try = 0;
    uint16_t cir_cnt, arc_cnt;
    struct s_XY in[3];
    struct s_XY max, min;
    struct s_cir_out cir_out = {0, 0, 0};
    struct s_arc_out arc_out = {0, {0, 0}, {0, 0}};

    if(OV7670_List.index < 30 || OV7670_List.index > 200) {
        return;
    }

    do {
        // get 3 random point
        for(int i = 0; i < 3; i++) {
            rand = RNG_RandomIn(OV7670_List.index);
            in[i].x = OV7670_List.pos[rand].x;
            in[i].y = OV7670_List.pos[rand].y;
        }

        x1 = in[1].x - in[0].x;
        y1 = in[1].y - in[0].y;
        x2 = in[2].x - in[0].x;
        y2 = in[2].y - in[0].y;

        // calculate circle center and radius
        r1 = x1 * x1 + y1 * y1;
        r2 = x2 * x2 + y2 * y2;
        x0 = (y2 * r1 - y1 * r2) / (2 * (x1 * y2 - x2 * y1));
        y0 = (x1 * r2 - x2 * r1) / (2 * (x1 * y2 - x2 * y1));
        r0 = x0 * x0 + x1 * x1;
        x0 = x0 + in[0].x;
        y0 = y0 + in[0].y;

        // calculate arcdratic function coefficient
        if(x1 != 0 && x2 != 0 && x1 - x2 != 0) {
            m1 = y1 / x1;
            m2 = y2 / x2;
            a = (m1 - m2) / (x1 - x2);
            b = (m2 * x1 - m1 * x2) / (x1 - x2);
        }

        // count in point
        cir_cnt = 0;
        arc_cnt = 0;
        min.x = GRAM_WIDTH;
        min.y = GRAM_HEIGHT;
        max.x = 0;
        max.y = 0;
        for(int i = 0; i < OV7670_List.index; i++) {
            // point fit circle
            dx = OV7670_List.pos[i].x - x0;
            dy = OV7670_List.pos[i].y - y0;
            dr = dx * dx + dy * dy;
            if(dr >= r0 - 4 && dr <= r0 + 4) {
                cir_cnt++;
            }
            // point fit arc
            dx = OV7670_List.pos[i].x - in[0].x;
            dy = OV7670_List.pos[i].y - in[0].y;
            dr = a * dx * dx + b * dx;
            if(dy >= dr - 2 && dy <= dr + 2) {
                arc_cnt++;
                if(OV7670_List.pos[i].x < min.x)
                    min.x = OV7670_List.pos[i].x;
                if(OV7670_List.pos[i].x > max.x)
                    max.x = OV7670_List.pos[i].x;
                if(OV7670_List.pos[i].y < min.y)
                    min.y = OV7670_List.pos[i].y;
                if(OV7670_List.pos[i].y > max.y)
                    max.y = OV7670_List.pos[i].y;
            }
        }
        if(cir_cnt > cir_out.cnt) {
            cir_out.cnt = cir_cnt;
            cir_out.x = x0;
            cir_out.y = y0;
        }
        if(arc_cnt > arc_out.cnt) {
            arc_out.cnt = arc_cnt;
            arc_out.min.x = min.x;
            arc_out.min.y = min.y;
            arc_out.max.x = max.x;
            arc_out.max.y = max.y;
        }
    } while(try++ < 400);

    if(cir_out.cnt >= 20) {
        OLED_TargetX(cir_out.x, cir_out.y);
    }
    if(arc_out.cnt >= 50) {
        OLED_Square(arc_out.min.x, arc_out.min.y, arc_out.max.x, arc_out.max.y);
    }
    // printf("%d,%d,%d,%d,%d,%d\r\n",cir_out.x, cir_out.y, arc_out.min.x, arc_out.min.y, arc_out.max.x, arc_out.max.y);
}

void GaussianFilter(void) {
    int32_t tmp;
    for(int y = 0; y < OV7670_HEIGHT; y++) {
        for(int x = 0; x < OV7670_WIDTH; x++) {
            if(x == 0 || x == OV7670_WIDTH - 1 || y == 0 || y == OV7670_HEIGHT - 1) {
                OV7670_Buf[y][x].Layer1 = OV7670_Buf[y][x].Layer0;
            } else {
                tmp =   OV7670_Buf[y-1][x-1].Layer0*1 +
                        OV7670_Buf[y-1][x  ].Layer0*2 +
                        OV7670_Buf[y-1][x+1].Layer0*1 +
                        OV7670_Buf[y  ][x-1].Layer0*2 +
                        OV7670_Buf[y  ][x  ].Layer0*4 +
                        OV7670_Buf[y  ][x+1].Layer0*2 +
                        OV7670_Buf[y+1][x-1].Layer0*1 +
                        OV7670_Buf[y+1][x  ].Layer0*2 +
                        OV7670_Buf[y+1][x+1].Layer0*1;
                OV7670_Buf[y][x].Layer1 = tmp / 16;
            }
        }
    }
}

void EdgeDetection(void) {
    int32_t tmp;
    OV7670_List.index = 0;
    for(int y = 0; y < OV7670_HEIGHT; y++) {
        for(int x = 0; x < OV7670_WIDTH; x++) {
            OV7670_Buf[y][x].Layer0 = 0;
            if(x != 0 && x != OV7670_WIDTH - 1 && y != 0 && y != OV7670_HEIGHT - 1) {
                tmp = -(OV7670_Buf[y][x].Layer1*4 -
                        OV7670_Buf[y-1][x].Layer1 -
                        OV7670_Buf[y][x-1].Layer1 -
                        OV7670_Buf[y][x+1].Layer1 -
                        OV7670_Buf[y+1][x].Layer1);
                if(tmp >= grayThreshold) {
                    OV7670_Buf[y-1][x].Layer0 |= 0x1;
                    OV7670_Buf[y][x-1].Layer0 |= 0x1;
                    OV7670_Buf[y][x+1].Layer0 |= 0x1;
                    OV7670_Buf[y+1][x].Layer0 |= 0x1;
                    OLED_Data.GRAM[y / 8][x] |= 1 << y % 8;
                    OV7670_List.pos[OV7670_List.index].x = x;
                    OV7670_List.pos[OV7670_List.index].y = y;
                    OV7670_List.index++;
                } else if(tmp >= grayThreshold - 0x04) {
                    OV7670_Buf[y][x].Layer0 |= 0x2;
                } else {
                    OLED_Data.GRAM[y / 8][x] &= ~(1 << y % 8);
                }
            }
            if(y >= 2 && x >= 1 && (OV7670_Buf[y-1][x].Layer0 & 0x3) == 0x3) {
                OLED_Data.GRAM[(y - 1) / 8][x] |= 1 << (y - 1) % 8;
                OV7670_List.pos[OV7670_List.index].x = x;
                OV7670_List.pos[OV7670_List.index].y = y - 1;
                OV7670_List.index++;
            }
        }
    }
}