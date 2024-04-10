#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include "myDriver_tim.h"

#define SIC_H() GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define SIC_L() GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define SID_H() GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define SID_L() GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define SID_R() GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)

void SCCB_Init(void) {
    GPIO_InitTypeDef hGPIOB;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOBEN, ENABLE);

    // Config PB8 as SCL, PB9 as SDA
    hGPIOB.GPIO_Mode =  GPIO_Mode_OUT;
    hGPIOB.GPIO_OType = GPIO_OType_OD;
    hGPIOB.GPIO_Pin =   GPIO_Pin_8 | GPIO_Pin_9;
    hGPIOB.GPIO_PuPd =  GPIO_PuPd_UP;
    hGPIOB.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
    GPIO_Init(GPIOB, &hGPIOB);
}

void SCCB_Start(void) {
    SIC_H();
    SID_H();
    delay_sccb(1);
    SID_L();
    delay_sccb(1);
    SIC_L();
}

void SCCB_Stop(void) {
    SID_L();
    delay_sccb(1);
    SIC_H();
    delay_sccb(1);
    SID_H();
}

uint8_t SCCB_WriteByte(uint8_t byte) {
    uint8_t ack = 0;
    for(int i = 0; i < 8; i++) {
        if((byte << i) & 0x80) {
            SID_H();
        } else {
            SID_L();
        }
        delay_sccb(1);
        SIC_H();
        delay_sccb(1);
        SIC_L();
    }
    delay_sccb(1);
    SIC_H();
    ack = SID_R();  // Dont-Care bit
    delay_sccb(1);
    SIC_L();

    return ack;
}

uint8_t SCCB_ReadByte(void) {
    uint8_t byte = 0;
    for(int i = 0; i < 8; i++) {
        delay_sccb(1);
        SIC_H();
        if(SID_R()) {
            byte |= 0x80 >> i;
        } else {
            byte &= ~(0x80 >> i);
        }
        delay_sccb(1);
        SIC_L();
    }
    SID_H();
    delay_sccb(1);
    SIC_H();
    delay_sccb(1);
    SIC_L();

    return byte;
}

void SCCB_WriteReg(uint8_t addr, uint8_t reg, uint8_t val) {
    uint8_t ack = 0;

    // 3 Phase Write
    SCCB_Start();
    ack = SCCB_WriteByte(addr & ~0x01);
    ack = SCCB_WriteByte(reg);
    ack = SCCB_WriteByte(val);
    SCCB_Stop();
}

uint8_t SCCB_ReadReg(uint8_t addr, uint8_t reg) {
    uint8_t ack = 0;

    // 2 Phase Write
    SCCB_Start();
    ack = SCCB_WriteByte(addr & ~0x01);
    ack = SCCB_WriteByte(reg);
    SCCB_Stop();

    // 2 Phase Read
    SCCB_Start();
    ack = SCCB_WriteByte(addr | 0x01);
    ack = SCCB_ReadByte();
    SCCB_Stop();

    return ack;
}
