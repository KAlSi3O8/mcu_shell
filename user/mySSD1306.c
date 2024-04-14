#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <string.h>
#include "myDriver_iic.h"
#include "myDriver_tim.h"
#include "mySSD1306.h"

#define OLED_ADDR           (0x78)
#define OLED_CTR_ALLCMD     (0x00)
#define OLED_CTR_ALLDATA    (0x40)
#define OLED_CTR_ONECMD     (0x80)
#define OLED_CTR_ONEDATA    (0xC0)
#define OLED_INITCMD "\xAE\
\xA8\x3F\xD3\x00\x40\x2E\
\xA0\xC0\xDA\x12\x81\x7F\
\xA4\xA6\xD5\x80\x20\x00\
\x21\x20\x5F\x22\x00\x05\
\x8D\x14\xAF"
#define OLED_ALL    "\x80\x21\x80\x20\x80\x5F\x80\x22\x80\x00\x80\x05\x40"

struct s_GRAM OLED_Data = {.ctrl = 0x40};

void OLED_nCmd(uint8_t *cmd, uint32_t len) {
    IIC_WriteBytes(OLED_ADDR, OLED_CTR_ALLCMD, cmd, len);
}

void OLED_nData(uint8_t *data, uint32_t len) {
    IIC_DMAWriteBytes(OLED_ADDR, data, len);
}

void OLED_Init(void) {
    OLED_nCmd(OLED_INITCMD, sizeof(OLED_INITCMD));
    // OLED_SetSize(64, 48);
    OLED_Fill(0x00);
}

void OLED_SetSize(uint8_t width, uint8_t height) {
    uint8_t cmd[6] = {0x21, 0x20, 0x00, 0x22, 0x00, 0x00};
    cmd[2] = 0x1F + width;
    cmd[5] = height == 0 ? 0 : (height - 1) / 8;
    OLED_nCmd(cmd, sizeof(cmd));
}

void OLED_Fill(uint8_t byte) {
    memset(OLED_Data.GRAM, byte, GRAM_MAX_SIZE);
    OLED_nData((uint8_t *)&OLED_Data, sizeof(OLED_Data));
}

void OLED_Flush(void) {
    OLED_nData((uint8_t *)&OLED_Data, sizeof(OLED_Data));
}