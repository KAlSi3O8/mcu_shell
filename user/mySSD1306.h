#ifndef __MYSSD1306_H
#define __MYSSD1306_H

#define GRAM_SIZE   384

struct s_GRAM {
    uint8_t ctrl;
    uint8_t GRAM[6][64];
};
extern struct s_GRAM OLED_Data;

void OLED_Init(void);
void OLED_Fill(uint8_t byte);
void OLED_Flush(void);

#endif