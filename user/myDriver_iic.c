#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include "mySSD1306.h"
#include "myDriver_tim.h"

void IIC_Init(void) {
    GPIO_InitTypeDef hGPIOF;
    I2C_InitTypeDef hI2C2;
    DMA_InitTypeDef hDMA1;
    NVIC_InitTypeDef hNVIC;

    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOFEN, ENABLE);
    // Config PF0 as SDA, PF1 as SCL
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_I2C2);
    hGPIOF.GPIO_Mode    = GPIO_Mode_AF;
    hGPIOF.GPIO_OType   = GPIO_OType_OD;
    hGPIOF.GPIO_Pin     = GPIO_Pin_0 | GPIO_Pin_1;
    hGPIOF.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    hGPIOF.GPIO_Speed   = GPIO_High_Speed;
    GPIO_Init(GPIOF, &hGPIOF);

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C2EN, ENABLE);
    hI2C2.I2C_Mode      = I2C_Mode_I2C;
    hI2C2.I2C_Ack       = I2C_Ack_Enable;
    hI2C2.I2C_DutyCycle = I2C_DutyCycle_2;
    hI2C2.I2C_ClockSpeed= 400000;
    hI2C2.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &hI2C2);

    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA1EN, ENABLE);
    hDMA1.DMA_Channel       = DMA_Channel_7;
    hDMA1.DMA_PeripheralBaseAddr = &I2C2->DR;
    hDMA1.DMA_Memory0BaseAddr    = 0;
    hDMA1.DMA_DIR           = DMA_DIR_MemoryToPeripheral;
    hDMA1.DMA_BufferSize    = 0;
    hDMA1.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    hDMA1.DMA_MemoryInc     = DMA_MemoryInc_Enable;
    hDMA1.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    hDMA1.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    hDMA1.DMA_Mode          = DMA_Mode_Normal;
    hDMA1.DMA_Priority      = DMA_Priority_High;
    hDMA1.DMA_FIFOMode      = DMA_FIFOMode_Disable;
    hDMA1.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    hDMA1.DMA_MemoryBurst   = DMA_MemoryBurst_Single;
    hDMA1.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream7, &hDMA1);
    DMA_ITConfig(DMA1_Stream7, DMA_IT_TC, ENABLE);

    hNVIC.NVIC_IRQChannel = DMA1_Stream7_IRQn;
    hNVIC.NVIC_IRQChannelCmd = ENABLE;
    hNVIC.NVIC_IRQChannelPreemptionPriority = 0;
    hNVIC.NVIC_IRQChannelSubPriority = 10;
    NVIC_Init(&hNVIC);

    I2C_Cmd(I2C2, ENABLE);
}

void IIC_DMAWriteBytes(uint8_t addr, uint8_t *byte, uint32_t size) {
    DMA_MemoryTargetConfig(DMA1_Stream7, (uint32_t)byte, DMA_Memory_0);
    DMA_SetCurrDataCounter(DMA1_Stream7, size);
    DMA_Cmd(DMA1_Stream7, ENABLE);
    I2C_DMACmd(I2C2, ENABLE);

    I2C_GenerateSTART(I2C2, ENABLE);
    while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
    // delay_ms(1);
    // while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT)) {
    //     delay_ms(10);
    //     printf("SR=%04x%04x\r\n", I2C2->SR2, I2C2->SR1);
    // }
    I2C_Send7bitAddress(I2C2, addr, I2C_Direction_Transmitter);
    while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    // delay_ms(1);
    // while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    //     delay_ms(10);
    //     printf("SR=%04x%04x\r\n", I2C2->SR2, I2C2->SR1);
    // }
}

void IIC_WriteBytes(uint8_t addr, uint8_t ctl, uint8_t *byte, uint32_t size) {
    I2C_GenerateSTART(I2C2, ENABLE);
    while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C2, addr, I2C_Direction_Transmitter);
    while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C2, ctl);
    while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
    while(size > 0) {
        I2C_SendData(I2C2, *byte);
        while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
        byte++;
        size--;
    }
    while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C2, ENABLE);
}

void DMA1_Stream7_IRQHandler(void) {
    if(DMA_GetITStatus(DMA1_Stream7, DMA_IT_TCIF7)) {
        DMA_ClearITPendingBit(DMA1_Stream7, DMA_IT_TCIF7);
        while(0 == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        I2C_GenerateSTOP(I2C2, ENABLE);
        DMA_Cmd(DMA1_Stream7, DISABLE);
        I2C_DMACmd(I2C2, DISABLE);
    }
}