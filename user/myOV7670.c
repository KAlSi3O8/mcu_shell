#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "myDriver_sccb.h"
#include "myDriver_tim.h"
#include "mySSD1306.h"
#include "myOV7670.h"
#include "myRANSAC.h"

#define VREF    0x03
#define COM1    0x04
#define AECHH   0x07
#define PID     0x0A
#define VER     0x0B
#define COM3    0x0C
#define AECH    0x10
#define CLKRC   0x11
#define COM7    0x12
#define COM9    0x14
#define HSTART  0x17
#define HSTOP   0x18
#define VSTART  0x19
#define VSTOP   0x1A
#define MIDH    0x1C
#define MIDL    0x1D
#define MVFC    0x1E
#define HREF    0x32
#define COM14   0x3E
#define DCWCTR  0x72
#define PCLKDIV 0x73

struct s_buf OV7670_Buf[OV7670_HEIGHT][OV7670_WIDTH] = {0};
uint8_t grayThreshold = 0x10;

uint16_t OV7670_GetMID(void) {
    uint16_t MID = 0;
    MID = (SCCB_ReadReg(OV7670_ADDR, MIDH) << 8) |
            (0xFF & SCCB_ReadReg(OV7670_ADDR, MIDL));
    return MID;
}

uint16_t OV7670_GetPID(void) {
    uint16_t ID = 0;
    ID = (SCCB_ReadReg(OV7670_ADDR, PID) << 8) |
            (0xFF & SCCB_ReadReg(OV7670_ADDR, VER));
    return ID;
}

uint16_t OV7670_GetAEC(void) {
    uint16_t AEC = 0;
    AEC =   ((SCCB_ReadReg(OV7670_ADDR, AECHH) & 0x3F) << 10) |
            (SCCB_ReadReg(OV7670_ADDR, AECH) << 1) |
            (SCCB_ReadReg(OV7670_ADDR, COM1) & 0x03);
    return AEC;
}

void OV7670_SetSize(uint16_t width, uint16_t height) {
    uint8_t Hstart;
    uint8_t Hstop;
    uint8_t Hlow;
    uint8_t Vstart;
    uint8_t Vstop;
    uint8_t Vlow;

    Hstart = 0x39 - width / 16;
    Hstop  = 0x39 + width / 16;
    Hlow   = (8 - ((width / 2) % 8)) & 0x7 | (((width / 2) % 8) & 0x7) << 3 | 0x80;
    Vstart = 0x3F - height / 8;
    Vstop  = 0x3F + height / 8;
    Vlow   = (4 - ((height / 2) % 4)) & 0x3 | (((height / 2) % 4) & 0x3) << 2;

    SCCB_WriteReg(OV7670_ADDR, HSTART, Hstart);
    SCCB_WriteReg(OV7670_ADDR, HSTOP, Hstop);
    SCCB_WriteReg(OV7670_ADDR, HREF, Hlow);
    SCCB_WriteReg(OV7670_ADDR, VSTART, Vstart);
    SCCB_WriteReg(OV7670_ADDR, VSTOP, Vstop);
    SCCB_WriteReg(OV7670_ADDR, VREF, Vlow);
}

void OV7670_SetDownSampling(uint8_t rate) {
    switch (rate)
    {
    case 2:
        rate = 0x1;
        break;
    case 4:
        rate = 0x2;
        break;
    case 8:
        rate = 0x3;
        break;
    default:
        return;
    }

    SCCB_WriteReg(OV7670_ADDR, COM3, 0x04);             // Enable down sampling
    SCCB_WriteReg(OV7670_ADDR, COM14, 0x10 | rate);      // Enable down sampling PCLK control, set PCLK divider /4
    SCCB_WriteReg(OV7670_ADDR, DCWCTR, rate << 4 | rate); // down sample /4
    SCCB_WriteReg(OV7670_ADDR, PCLKDIV, rate);             // clock divider /4 (= PCLK divider)
}

int OV7670_SoftReset(void) {
    delay_ms(5);
    SCCB_WriteReg(OV7670_ADDR, COM7, 0x80);
    delay_ms(5);
    if(0x7673 != OV7670_GetPID()) {
        printf("Failed to get correct PID: 0x%0x\r\n", OV7670_GetPID());
        return -1;
    }

    SCCB_WriteReg(OV7670_ADDR, COM7, 0x00);     // Output format YUV
    SCCB_WriteReg(OV7670_ADDR, COM9, 0x1a);
    SCCB_WriteReg(OV7670_ADDR, CLKRC, 0x81);    // pre-scale by 4
    SCCB_WriteReg(OV7670_ADDR, MVFC, 0x20);     // mirror horizen
    OV7670_SetSize(OV7670_WIDTH * OV7670_RATE, OV7670_HEIGHT * OV7670_RATE);
    OV7670_SetDownSampling(OV7670_RATE);
    return 0;
}

int OV7670_Init(void) {
    GPIO_InitTypeDef hGPIO;
    DCMI_InitTypeDef hDCMI;
    NVIC_InitTypeDef hNVIC;
    DMA_InitTypeDef  hDMA2;

    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOBEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOEEN, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2ENR_DCMIEN,  ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN,  ENABLE);

    hGPIO.GPIO_Mode = GPIO_Mode_AF;
    hGPIO.GPIO_OType = GPIO_OType_PP;
    hGPIO.GPIO_PuPd = GPIO_PuPd_NOPULL;
    hGPIO.GPIO_Speed = GPIO_High_Speed;

    hGPIO.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI);     // HSYNC
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI);     // PIXCLK
    GPIO_Init(GPIOA, &hGPIO);

    hGPIO.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_DCMI);     // D5
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI);     // VSYNC
    GPIO_Init(GPIOB, &hGPIO);

    hGPIO.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_DCMI);     // D0
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI);     // D1
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_DCMI);     // D2
    GPIO_Init(GPIOC, &hGPIO);

    hGPIO.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_DCMI);     // D3
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_DCMI);     // D4
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_DCMI);     // D6
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_DCMI);     // D7
    GPIO_Init(GPIOE, &hGPIO);

    hDCMI.DCMI_CaptureMode = DCMI_CaptureMode_Continuous;
    // hDCMI.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;
    hDCMI.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
    hDCMI.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;
    hDCMI.DCMI_VSPolarity  = DCMI_VSPolarity_High;
    hDCMI.DCMI_HSPolarity  = DCMI_HSPolarity_Low;
    hDCMI.DCMI_CaptureRate = DCMI_CaptureRate_1of2_Frame;
    hDCMI.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
    DCMI_Init(&hDCMI);
    DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);

    hDMA2.DMA_Channel = DMA_Channel_1;
    hDMA2.DMA_PeripheralBaseAddr = &(DCMI->DR);
    hDMA2.DMA_Memory0BaseAddr = OV7670_Buf;
    hDMA2.DMA_DIR = DMA_DIR_PeripheralToMemory;
    hDMA2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    hDMA2.DMA_MemoryInc = DMA_MemoryInc_Enable;
    hDMA2.DMA_BufferSize = OV7670_BUF_SIZE / 2;
    hDMA2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    hDMA2.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    hDMA2.DMA_Mode = DMA_Mode_Circular;
    hDMA2.DMA_Priority = DMA_Priority_High;
    hDMA2.DMA_FIFOMode = DMA_FIFOMode_Disable;
    hDMA2.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    hDMA2.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    hDMA2.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream1, &hDMA2);
    DMA_Cmd(DMA2_Stream1, ENABLE);

    hNVIC.NVIC_IRQChannel = DCMI_IRQn;
    hNVIC.NVIC_IRQChannelPreemptionPriority = 1;
    hNVIC.NVIC_IRQChannelSubPriority = 7;
    hNVIC.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&hNVIC);

    DCMI_Cmd(ENABLE);
}

void DCMI_IRQHandler(void) {
    int tmp;
    if(DCMI_GetITStatus(DCMI_IT_FRAME)) {
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);
        DMA_Cmd(DMA2_Stream1, DISABLE);
        DMA_SetCurrDataCounter(DMA2_Stream1, OV7670_BUF_SIZE / 2);
        DMA_Cmd(DMA2_Stream1, ENABLE);

        // Gaussian filter
        GaussianFilter();
        // for(int y = 0; y < OV7670_HEIGHT; y++) {
        //     for(int x = 0; x < OV7670_WIDTH; x++) {
        //         if(x == 0 || x == OV7670_WIDTH - 1 || y == 0 || y == OV7670_HEIGHT - 1) {
        //             OV7670_Buf[y][x].Layer1 = 0;
        //         } else {
        //             tmp =   OV7670_Buf[y-1][x-1].Layer0*1 +
        //                     OV7670_Buf[y-1][x  ].Layer0*2 +
        //                     OV7670_Buf[y-1][x+1].Layer0*1 +
        //                     OV7670_Buf[y  ][x-1].Layer0*2 +
        //                     OV7670_Buf[y  ][x  ].Layer0*4 +
        //                     OV7670_Buf[y  ][x+1].Layer0*2 +
        //                     OV7670_Buf[y+1][x-1].Layer0*1 +
        //                     OV7670_Buf[y+1][x  ].Layer0*2 +
        //                     OV7670_Buf[y+1][x+1].Layer0*1;
        //             OV7670_Buf[y][x].Layer1 = tmp / 16;
        //         }
        //     }
        // }

        // Edge detection and binarization
        EdgeDetection();
        // OV7670_List.index = 0;
        // for(int y = 0; y < OV7670_HEIGHT; y++) {
        //     for(int x = 0; x < OV7670_WIDTH; x++) {
        //         if(x == 0 || x == OV7670_WIDTH - 1 || y == 0 || y == OV7670_HEIGHT - 1) {
        //             OV7670_Buf[y][x].Layer0 = 0;
        //         } else {
        //             tmp =   OV7670_Buf[y][x].Layer1*4 -
        //                     OV7670_Buf[y-1][x].Layer1 -
        //                     OV7670_Buf[y][x-1].Layer1 -
        //                     OV7670_Buf[y][x+1].Layer1 -
        //                     OV7670_Buf[y+1][x].Layer1;
        //             if(-tmp >= grayThreshold) {
        //                 OLED_Data.GRAM[y / 8][x] |= 1 << y % 8;
        //                 OV7670_List.pos[OV7670_List.index].x = x;
        //                 OV7670_List.pos[OV7670_List.index].y = y;
        //                 OV7670_List.index++;
        //             } else {
        //                 OLED_Data.GRAM[y / 8][x] &= ~(1 << y % 8);
        //             }
        //         }
        //     }
        // }

        RANSAC();
        OLED_Flush();
    }
}