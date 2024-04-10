#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include "myDriver_sccb.h"
#include "myDriver_tim.h"
#include "mySSD1306.h"

#define OV7670_ADDR 0x42
#define OV7670_BUF_SIZE     3072

#define COM7    0x12
#define PID     0x0A
#define VER     0x0B
#define MIDH    0x1C
#define MIDL    0x1D

struct s_YCbCr {
    uint8_t CbCr;
    uint8_t Y;
} OV7670_Buf[OV7670_BUF_SIZE] = {0};

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

int OV7670_SoftReset(void) {
    delay_ms(5);
    SCCB_WriteReg(OV7670_ADDR, COM7, 0x80);
    delay_ms(5);
    if(0x7673 != OV7670_GetPID()) {
        printf("Failed to get correct PID: 0x%0x\r\n", OV7670_GetPID());
        return -1;
    }

    SCCB_WriteReg(OV7670_ADDR, 0x17, 0x35);     // set to 64x48
    SCCB_WriteReg(OV7670_ADDR, 0x18, 0x3D);
    SCCB_WriteReg(OV7670_ADDR, 0x32, 0x00);
    SCCB_WriteReg(OV7670_ADDR, 0x19, 0x39);
    SCCB_WriteReg(OV7670_ADDR, 0x1A, 0x45);
    SCCB_WriteReg(OV7670_ADDR, 0x03, 0x00);

    // SCCB_WriteReg(OV7670_ADDR, 0x11, 0x01);     // pre-scalar /4
    SCCB_WriteReg(OV7670_ADDR, 0x12, 0x00);     // Output format YUV

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
    hDCMI.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
    hDCMI.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;
    hDCMI.DCMI_VSPolarity  = DCMI_VSPolarity_High;
    hDCMI.DCMI_HSPolarity  = DCMI_HSPolarity_Low;
    hDCMI.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
    hDCMI.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
    DCMI_Init(&hDCMI);
    DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);

    hDMA2.DMA_Channel = DMA_Channel_1;
    hDMA2.DMA_PeripheralBaseAddr = &(DCMI->DR);
    hDMA2.DMA_Memory0BaseAddr = OV7670_Buf;
    hDMA2.DMA_DIR = DMA_DIR_PeripheralToMemory;
    hDMA2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    hDMA2.DMA_MemoryInc = DMA_MemoryInc_Enable;
    hDMA2.DMA_BufferSize = 1536;
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
    hNVIC.NVIC_IRQChannelPreemptionPriority = 0;
    hNVIC.NVIC_IRQChannelSubPriority = 10;
    hNVIC.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&hNVIC);

    DCMI_Cmd(ENABLE);
}

void DCMI_IRQHandler(void) {
    if(DCMI_GetITStatus(DCMI_IT_FRAME)) {
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);
        DMA_Cmd(DMA2_Stream1, DISABLE);
        DMA_SetCurrDataCounter(DMA2_Stream1, 1536);
        DMA_Cmd(DMA2_Stream1, ENABLE);
        for(int y = 0; y < 48; y++) {
            for(int x = 0; x < 64; x++) {
                if(OV7670_Buf[x + y * 64].Y >= 0x80) {
                    OLED_Data.GRAM[y/8][x] |= 1 << y%8;
                } else {
                    OLED_Data.GRAM[y/8][x] &= ~(1 << y%8);
                }
            }
        }
        OLED_Flush();
    }
}