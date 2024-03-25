#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myDriver_uart.h"
#include "myDriver_tim.h"
#include "myDriver_sccb.h"
#include "myDriver_system.h"
#include "myApp.h"

int main(void) {
    int32_t ready_slot;
    uint8_t PID = 0;
    system_start();

    TIM6_Init();
    MCO2_Init();
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
