#include "myDriver_rng.h"
#include "mySSD1306.h"
#include "myRANSAC.h"

struct s_pos OV7670_List;

void RANSAC(void) {
    int32_t x0, y0, r0;
    int32_t x1, y1, r1;
    int32_t x2, y2, r2;
    int32_t dx, dy, dr;
    uint32_t rand, in_cnt, try = 0;
    struct s_XY in[3];

    if(OV7670_List.index < 40) {
        return;
    }

    do {
        // get 3 random point
        for(int i = 0; i < 3; i++) {
            rand = RNG_RandomIn(OV7670_List.index);
            in[i].x = OV7670_List.pos[rand].x;
            in[i].y = OV7670_List.pos[rand].y;
        }

        // calculate circle center and radius
        x1 = in[1].x - in[0].x;
        y1 = in[1].y - in[0].y;
        r1 = x1 * x1 + y1 * y1;
        x2 = in[2].x - in[0].x;
        y2 = in[2].y - in[0].y;
        r2 = x2 * x2 + y2 * y2;
        x0 = (y2 * r1 - y1 * r2) / (2 * (x1 * y2 - x2 * y1));
        y0 = (x1 * r2 - x2 * r1) / (2 * (x1 * y2 - x2 * y1));
        r0 = x0 * x0 + x1 * x1;
        x0 = x0 + in[0].x;
        y0 = y0 + in[0].y;

        // count in point
        in_cnt = 0;
        for(int i = 0; i < OV7670_List.index; i++) {
            dx = OV7670_List.pos[i].x - x0;
            dy = OV7670_List.pos[i].y - y0;
            dr = dx * dx + dy * dy;
            if(dr > r0 - 8 && dr < r0 + 8) {
                in_cnt++;
            }
        }
    } while(in_cnt < 30 && try++ < 200);

    if(in_cnt >= 30) {
        OLED_TargetX(x0, y0);
        printf("(%d,%d)[%d], in:%d, try:%d\r\n", x0, y0, r0, in_cnt, try);
    }
}