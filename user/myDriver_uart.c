#include <stm32f4xx_conf.h>
#include "myDriver_uart.h"

void usart1_init(void) {
    GPIO_InitTypeDef hGPIOA;
    USART_InitTypeDef hUSART1;

    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    hGPIOA.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    hGPIOA.GPIO_Mode = GPIO_Mode_AF;
    hGPIOA.GPIO_Speed = GPIO_Speed_50MHz;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &hGPIOA);

    USART_StructInit(&hUSART1);
    hUSART1.USART_BaudRate = 115200;
    USART_Init(USART1, &hUSART1);
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