#include <stm32f4xx_conf.h>
#include "myDriver_uart.h"
#include <string.h>

static uint8_t uart1_dma_bufa[DMA_BUFFER_SIZE];
static uint8_t uart1_dma_bufb[DMA_BUFFER_SIZE];

void usart1_dma_init(void) {
    DMA_InitTypeDef hDMA2_S5;
    NVIC_InitTypeDef hNVIC;

    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN, ENABLE);

    DMA_StructInit(&hDMA2_S5);
    hDMA2_S5.DMA_Channel = DMA_Channel_4;
    hDMA2_S5.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    hDMA2_S5.DMA_Memory0BaseAddr = (uint32_t)uart1_dma_bufa;
    hDMA2_S5.DMA_DIR = DMA_DIR_PeripheralToMemory;
    hDMA2_S5.DMA_BufferSize = DMA_BUFFER_SIZE;
    DMA_Init(DMA2_Stream5, &hDMA2_S5);
    DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
    DMA_DoubleBufferModeConfig(DMA2_Stream5, (uint32_t)uart1_dma_bufb, DMA_Memory_0);

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

// Config Rx DMA
    usart1_dma_init();

    USART_Cmd(USART1, ENABLE);
}

void usart1_sendstr(char *str) {
    int len = strlen(str);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    for(int i = 0; i < len; i++) {
        USART_SendData(USART1, str[i]);
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    return;
}

void USART1_IRQHandler(void) {
    
}

void DMA2_Stream5_IRQHandler(void) {
    uint32_t memoryslot;
    char buf[32] = {0};
    if(DMA_GetITStatus(DMA2_Stream5, DMA_IT_TC)) {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TC);
        memoryslot = DMA_GetCurrentMemoryTarget(DMA2_Stream5);
    }
}