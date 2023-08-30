#include <stm32f4xx_conf.h>
#include "myDriver_uart.h"
#include <stdio.h>

static struct _uart1_dma_buf {
    uint8_t slot;
    uint8_t a[DMA_BUFFER_SIZE];
    uint8_t b[DMA_BUFFER_SIZE];
}uart1_dma_buf;

void usart1_dma_init(void) {
    DMA_InitTypeDef hDMA2_S5;
    NVIC_InitTypeDef hNVIC;

    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN, ENABLE);

    DMA_StructInit(&hDMA2_S5);
    hDMA2_S5.DMA_Channel = DMA_Channel_4;
    hDMA2_S5.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    hDMA2_S5.DMA_Memory0BaseAddr = (uint32_t)uart1_dma_buf.a;
    hDMA2_S5.DMA_DIR = DMA_DIR_PeripheralToMemory;
    hDMA2_S5.DMA_BufferSize = DMA_BUFFER_SIZE;
    DMA_Init(DMA2_Stream5, &hDMA2_S5);
    DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
    DMA_DoubleBufferModeConfig(DMA2_Stream5, (uint32_t)uart1_dma_buf.b, DMA_Memory_0);
    DMA_DoubleBufferModeCmd(DMA2_Stream5, ENABLE);

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);
    hNVIC.NVIC_IRQChannel = DMA2_Stream5_IRQn;
    hNVIC.NVIC_IRQChannelCmd = ENABLE;
    hNVIC.NVIC_IRQChannelPreemptionPriority = 0;
    hNVIC.NVIC_IRQChannelSubPriority = 10;
    NVIC_Init(&hNVIC);

    DMA_Cmd(DMA2_Stream5, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    return;
}

void usart1_init(void) {
    GPIO_InitTypeDef hGPIOA;
    USART_InitTypeDef hUSART1;
    NVIC_InitTypeDef hNVIC;

// Enable Clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);

// Config GPIO PA9-TX PA10-RX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    hGPIOA.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    hGPIOA.GPIO_Mode = GPIO_Mode_AF;
    hGPIOA.GPIO_Speed = GPIO_Speed_50MHz;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &hGPIOA);

// Config Uart
    USART_StructInit(&hUSART1);
    hUSART1.USART_BaudRate = 115200;
    USART_Init(USART1, &hUSART1);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

// Config UART RX NVIC
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);
    hNVIC.NVIC_IRQChannel = USART1_IRQn;
    hNVIC.NVIC_IRQChannelCmd = ENABLE;
    hNVIC.NVIC_IRQChannelPreemptionPriority = 0;
    hNVIC.NVIC_IRQChannelSubPriority = 15;
    NVIC_Init(&hNVIC);

    USART_Cmd(USART1, ENABLE);
    return;
}

void usart1_sendstr(char *str) {
    int len = strlen(str);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    for(int i = 0; i < len; i++) {
        USART_SendData(USART1, str[i]);
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    return;
}

void USART1_IRQHandler(void) {
    if(USART_GetITStatus(USART1, USART_IT_IDLE)) {
        USART1->SR;
        USART1->DR;
        USART_SendData(USART1, USART_ReceiveData(USART1));
        // if(0 == DMA_GetCurrentMemoryTarget(DMA2_Stream5)) {
        //     DMA2_Stream5->CR |= (uint32_t)DMA_SxCR_CT;
        //     usart1_sendstr("[USART1]memory slot --> 0\r\ndata -->\r\n");
        //     usart1_sendstr(uart1_dma_buf.a);
        //     usart1_sendstr("\r\n[USART1]switch to slot 1\r\n\n");
        // } else {
        //     DMA2_Stream5->CR &= ~(uint32_t)DMA_SxCR_CT;
        //     usart1_sendstr("[USART1]memory slot --> 1\r\ndata -->\r\n");
        //     usart1_sendstr(uart1_dma_buf.b);
        //     usart1_sendstr("\r\n[USART1]switch to slot 0\r\n\n");
        // }
    }
    return;
}

void DMA2_Stream5_IRQHandler(void) {
    if(DMA_GetITStatus(DMA2_Stream5, DMA_IT_TC)) {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TC);
        if(DMA_GetCurrentMemoryTarget(DMA2_Stream5) == 0) {
            usart1_sendstr("[DMA2_S5]memory slot --> 0\r\ndata -->\r\n");
            usart1_sendstr(uart1_dma_buf.a);
            usart1_sendstr("\r\n[DMA2_S5]switch to slot 1\r\n\n");
        } else {
            usart1_sendstr("[DMA2_S5]memory slot --> 1\r\ndata -->\r\n");
            usart1_sendstr(uart1_dma_buf.b);
            usart1_sendstr("\r\n[DMA2_S5]switch to slot 0\r\n\n");
        }
    }
    return;
}