#ifndef __MYSSD1306_H
#define __MYSSD1306_H

#define GRAM_WIDTH  32
#define GRAM_HEIGHT 24
#define GRAM_SIZE   (GRAM_WIDTH * GRAM_HEIGHT / 8)
#define GRAM_MAX_SIZE   384

struct s_GRAM {
    uint8_t ctrl;
    uint8_t GRAM[GRAM_HEIGHT / 8][GRAM_WIDTH];
};
extern struct s_GRAM OLED_Data;

void OLED_Init(void);
void OLED_SetSize(uint8_t width, uint8_t height);
void OLED_Fill(uint8_t byte, uint32_t size);
void OLED_Flush(void);
void OLED_TargetX(uint8_t x, uint8_t y);
void OLED_Square(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

#endif