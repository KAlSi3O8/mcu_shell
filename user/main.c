#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
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

int get_token_size(char** str) {
    int size = 0;
    while(*(*str) != ' ' && *(*str) != '\r') {
        (*str)++;
        size++;
    }
    while(*(*str) == ' ') {
        (*str)++;
    }
    return size;
}

int devmem(char* args) {
    int arg_len;
    char* arg = args;
    char* tmp;
    uint32_t addr;
    uint32_t val;

    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        usart1_sendstr("Usage: devmem reg_addr [w value]\r\n");
        return 1;
    }
    tmp = (char*)malloc(arg_len + 1);
    strncpy(tmp, arg, arg_len);
    addr = strtoul(tmp+2, NULL, 16);
    val = *(__IO uint32_t*)addr;
    free(tmp);

    tmp = (char*)malloc(64);
    sprintf(tmp, "Value in 0x%08x is 0x%08x(%d)\r\n", addr, val, val);
    usart1_sendstr(tmp);

    return 0;
}

int process_cmd(char* cmd, int cmd_len) {
    int ret = 0;
    int token_len;
    char* token = cmd;

    token_len = get_token_size(&cmd);
    switch(token_len) {
        case 4:
            if(strncmp(token, "help", token_len) == 0) {
                usart1_sendstr("mcu shell cmd:\r\n");
                usart1_sendstr("devmem - show value of register\r\n");
                usart1_sendstr("help - show this info\r\n");
            } else {
                usart1_sendstr("Unknown cmd\r\n");
                usart1_sendstr("use \"help\" to show cmd info\r\n");
            }
            break;
        case 6:
            if(strncmp(token, "devmem", token_len) == 0) {
                ret = devmem(cmd);
            } else {
                usart1_sendstr("Unknown cmd\r\n");
                usart1_sendstr("use \"help\" to show cmd info\r\n");
            }
            break;
        default:
            usart1_sendstr("Unknown cmd\r\n");
            usart1_sendstr("use \"help\" to show cmd info\r\n");
            break;
    }

    return ret;
}

int main(void) {
    int32_t ready_slot;
    system_start();

    usart1_init();
    while(1) {
        ready_slot = usart1_get_ready_slot();
        if(ready_slot != -1) {
            process_cmd(uart1_dual_buf.buf[ready_slot], uart1_dual_buf.buf_len[ready_slot]);
            uart1_dual_buf.slot_flag &= ~SLOT_MSK(ready_slot);
        }
    }
}
