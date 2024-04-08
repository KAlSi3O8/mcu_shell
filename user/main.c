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
#include "myOV7670.h"

int main(void) {
    int32_t ready_slot;
    uint8_t PID = 0;
    system_start();

    UART1_Init();
    TIM6_Init();
    MCO2_Init();
    SCCB_Init();
    OV7670_SoftReset();
    OV7670_Init();

    printf("PID = 0x%4X\r\n", OV7670_GetPID());
    printf("MID = 0x%4X\r\n", OV7670_GetMID());

    delay_ms(2000);
    DCMI_CaptureCmd(ENABLE);

    while(1) {
        ready_slot = UART1_GetReadySlot();
        if(ready_slot != -1) {
            process_cmd(uart1_dual_buf.buf[ready_slot], uart1_dual_buf.buf_len[ready_slot]);
            uart1_dual_buf.slot_flag &= ~SLOT_MSK(ready_slot);
        }
    }
}
