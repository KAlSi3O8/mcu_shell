#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>

uint32_t last = 0;

void RNG_Init(void) {
    RCC_AHB2PeriphClockCmd(RCC_AHB2ENR_RNGEN, ENABLE);
    RNG_Cmd(ENABLE);
    while(RESET == RNG_GetFlagStatus(RNG_FLAG_DRDY));
    last = RNG_GetRandomNumber();
}

uint32_t RNG_RandomIn(uint32_t range) {
    uint32_t rand;
    do {
        while(RESET == RNG_GetFlagStatus(RNG_FLAG_DRDY));
        rand = RNG_GetRandomNumber();
    } while(rand == last);
    last = rand;
    return rand % range;
}