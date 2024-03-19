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
    GPIO_InitTypeDef hGPIOF;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOFEN, ENABLE);

    hGPIOF.GPIO_Mode = GPIO_Mode_OUT;
    hGPIOF.GPIO_OType = GPIO_OType_PP;
    hGPIOF.GPIO_Pin = GPIO_Pin_9;
    hGPIOF.GPIO_PuPd = GPIO_PuPd_UP;
    hGPIOF.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF, &hGPIOF);
    GPIO_ResetBits(GPIOF, GPIO_Pin_9);

    return;
}

void MCO1_Init(void) {
    GPIO_InitTypeDef hGPIOA;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);

    // Config PA8 as MCO
    GPIO_PinAFConfig(GPIOA, GPIO_Pin_8, GPIO_AF_MCO);
    hGPIOA.GPIO_Mode = GPIO_Mode_AF;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_Pin = GPIO_Pin_8;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_NOPULL;
    hGPIOA.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &hGPIOA);

    RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_4);
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
    __IO uint32_t* addr;
    __IO uint32_t val;

    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        usart1_sendstr("Usage: devmem reg_addr [w value]\r\n");
        return 1;
    }
    tmp = (char*)malloc(arg_len + 1);
    memset(tmp, 0, arg_len + 1);
    strncpy(tmp, arg, arg_len);
    addr = strtoul(tmp+2, NULL, 16);
    free(tmp);

    arg = args;
    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        tmp = (char*)malloc(64);
        memset(tmp, 0, 64);
        sprintf(tmp, "Value in 0x%08x is 0x%08x(%u)\r\n", addr, *addr, *addr);
        usart1_sendstr(tmp);
        free(tmp);
        return 0;
    }
    if(arg_len == 1 && arg[0] == 'w') {
        arg = args;
        usart1_sendstr(arg);
        arg_len = get_token_size(&args);
        if(arg_len == 0) {
            usart1_sendstr("lack of params\r\n");
            usart1_sendstr("Usage: devmem reg_addr [w value]\r\n");
            return -1;
        }

        tmp = (char*)malloc(arg_len + 1);
        memset(tmp, 0, arg_len + 1);
        strncpy(tmp, arg, arg_len);
        if(tmp[0] == '0' && tmp[1] == 'x') {
            val = strtoul(tmp+2, NULL, 16);
        } else {
            val = strtoul(tmp, NULL, 10);
        }
        free(tmp);

        tmp = (char*)malloc(64);
        memset(tmp, 0, 64);
        sprintf(tmp, "Wrote 0x%08x(%u) to 0x%08x\r\n", val, val, addr);
        usart1_sendstr(tmp);
        free(tmp);
        *addr = val;
    } else {
        usart1_sendstr("params worng\r\n");
        usart1_sendstr("Usage: devmem reg_addr [w value]\r\n");
        return -1;
    }

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

    MCO1_Init();
    usart1_init();
    while(1) {
        ready_slot = usart1_get_ready_slot();
        if(ready_slot != -1) {
            process_cmd(uart1_dual_buf.buf[ready_slot], uart1_dual_buf.buf_len[ready_slot]);
            uart1_dual_buf.slot_flag &= ~SLOT_MSK(ready_slot);
        }
    }
}
