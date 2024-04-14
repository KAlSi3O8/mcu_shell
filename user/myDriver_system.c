#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>



void MCO1_Init(void) {
    GPIO_InitTypeDef hGPIOA;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);

    // Config PA8 as MCO1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
    hGPIOA.GPIO_Mode = GPIO_Mode_AF;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_Pin = GPIO_Pin_8;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_NOPULL;
    hGPIOA.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOA, &hGPIOA);

    RCC_MCO1Config(RCC_MCO1Source_HSI, RCC_MCO1Div_1);
}

void MCO2_Init(void) {
    GPIO_InitTypeDef hGPIOC;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN, ENABLE);

    // Config PC9 as MCO2
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_MCO);
    hGPIOC.GPIO_Mode = GPIO_Mode_AF;
    hGPIOC.GPIO_OType = GPIO_OType_PP;
    hGPIOC.GPIO_Pin = GPIO_Pin_9;
    hGPIOC.GPIO_PuPd = GPIO_PuPd_NOPULL;
    hGPIOC.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOC, &hGPIOC);

    RCC_MCO2Config(RCC_MCO2Source_PLLI2SCLK, RCC_MCO2Div_3);
    RCC_PLLI2SConfig(72, 2);
    RCC_PLLI2SCmd(ENABLE);
    while(0 == RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY));
}