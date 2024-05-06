#ifndef __MYDRIVER_TIM_H
#define __MYDRIVER_TIM_H

void TIM6_Init(void);
void delay_ms(int ms);
void delay_sccb(int tick);

void TIM7_Init(void);
void TIM7_Start(void);
uint16_t TIM7_Get(void);
uint16_t TIM7_Stop(void);

#endif