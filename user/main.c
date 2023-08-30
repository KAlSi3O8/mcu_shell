#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <string.h>

#include "myDriver_uart.h"

void delay_ms(uint32_t time) {
    uint32_t cnt;
    while(time--) {
        for(cnt = 10000; cnt > 0; cnt--);
    }
    return;
}

void system_start(void) {
    GPIO_InitTypeDef hGPIOA;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);

    hGPIOA.GPIO_Mode = GPIO_Mode_OUT;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_Pin = GPIO_Pin_8;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_UP;
    hGPIOA.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &hGPIOA);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);

    return;
}

int main(void) {
    system_start();

    usart1_init();
    while(1) {
        usart1_sendstr("Hello World!\r\n");
        delay_ms(1000);
    }
}
