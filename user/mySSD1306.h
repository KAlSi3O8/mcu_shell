#ifndef __MYSSD1306_H
#define __MYSSD1306_H

#define GRAM_WIDTH  64
#define GRAM_HEIGHT 48
#define GRAM_SIZE   (GRAM_WIDTH * GRAM_HEIGHT)
#define GRAM_MAX_SIZE   384

struct s_GRAM {
    uint8_t ctrl;
    uint8_t GRAM[6][64];
};
extern struct s_GRAM OLED_Data;

void OLED_Init(void);
void OLED_SetSize(uint8_t width, uint8_t height);
void OLED_Fill(uint8_t byte);
void OLED_Flush(void);

#endif