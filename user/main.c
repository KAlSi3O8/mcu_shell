#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myDriver_uart.h"
#include "myDriver_tim.h"
#include "myDriver_sccb.h"
#include "myApp.h"

void MCO1_Init(void) {
    GPIO_InitTypeDef hGPIOA;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);

    // Config PA8 as MCO
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
    hGPIOA.GPIO_Mode = GPIO_Mode_AF;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_Pin = GPIO_Pin_8;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_NOPULL;
    hGPIOA.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOA, &hGPIOA);

    RCC_MCO1Config(RCC_MCO1Source_HSI, RCC_MCO1Div_1);
}

int main(void) {
    int32_t ready_slot;
    uint8_t PID = 0;
    system_start();

    TIM6_Init();
    MCO1_Init();
    SCCB_Init();
    UART1_Init();

    delay_ms(5);
    PID = SCCB_ReadReg(0x42, 0x0A);
    printf("PID: 0x%X\r\n", PID);
    PID = SCCB_ReadReg(0x42, 0x0B);
    printf("VER: 0x%X\r\n", PID);

    while(1) {
        D1_OFF();
        delay_ms(1000);
        D1_ON();
        delay_ms(1000);
    }

    while(1) {
        ready_slot = UART1_GetReadySlot();
        if(ready_slot != -1) {
            process_cmd(uart1_dual_buf.buf[ready_slot], uart1_dual_buf.buf_len[ready_slot]);
            uart1_dual_buf.slot_flag &= ~SLOT_MSK(ready_slot);
        }
    }
}
