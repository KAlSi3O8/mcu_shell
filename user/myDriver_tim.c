#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>

void TIM6_Init(void) {
    TIM_TimeBaseInitTypeDef hTIM6;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    hTIM6.TIM_Prescaler = 83;   // CK_CNT = 1MHz
    hTIM6.TIM_CounterMode = TIM_CounterMode_Down;
    hTIM6.TIM_Period =1000;    // ms delay

    TIM_TimeBaseInit(TIM6, &hTIM6);
    TIM_UpdateRequestConfig(TIM6, TIM_UpdateSource_Regular);
    TIM_Cmd(TIM6, ENABLE);
}

void delay_ms(int ms) {
    TIM_SetAutoreload(TIM6, 1000);
    TIM_GenerateEvent(TIM6, TIM_EventSource_Update);
    while(ms) {
        if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update)) {
            ms--;
            TIM_ClearFlag(TIM6, TIM_FLAG_Update);
        }
    }
}

void delay_sccb(int tick) {
    TIM_SetAutoreload(TIM6, 10);
    TIM_GenerateEvent(TIM6, TIM_EventSource_Update);
    while(tick) {
        if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update)) {
            tick--;
            TIM_ClearFlag(TIM6, TIM_FLAG_Update);
        }
    }
}