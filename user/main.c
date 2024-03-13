#include <stm32f4xx_conf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myos.h"
#include "myos_log.h"
#include "myDriver_uart.h"

#define LED_PORT GPIOF
#define LED_PIN  GPIO_Pin_9
#define LED_CLK  RCC_AHB1ENR_GPIOFEN

void delay_ms(uint32_t time) {
    uint32_t cnt;
    while(time--) {
        for(cnt = 10000; cnt > 0; cnt--);
    }
    return;
}

void system_start(void) {
    GPIO_InitTypeDef hLED;
    RCC_AHB1PeriphClockCmd(LED_CLK, ENABLE);

    hLED.GPIO_Mode = GPIO_Mode_OUT;
    hLED.GPIO_OType = GPIO_OType_PP;
    hLED.GPIO_Pin = LED_PIN;
    hLED.GPIO_PuPd = GPIO_PuPd_UP;
    hLED.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_PORT, &hLED);
    GPIO_ResetBits(LED_PORT, LED_PIN);

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);

    usart1_init();

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
    addr = (__IO uint32_t*)strtoul(tmp+2, NULL, 16);
    free(tmp);

    arg = args;
    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        tmp = (char*)malloc(64);
        memset(tmp, 0, 64);
        sprintf(tmp, "Value in 0x%08p is 0x%08lx(%lu)\r\n", addr, *addr, *addr);
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
        sprintf(tmp, "Wrote 0x%08lx(%lu) to 0x%08p\r\n", val, val, addr);
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

myos_STK LED_Stk[MYOS_TASK_STK_SIZE];
void LED_task(void *arg) {
    while(1) {
        GPIO_SetBits(LED_PORT, LED_PIN);
        myos_TimeDly(1000);
        GPIO_ResetBits(LED_PORT, LED_PIN);
        myos_TimeDly(1000);
    }
}

int main(void) {
    system_start();

    myos_Init();
    myos_TaskCreate(&LED_task, (void *)0, &LED_Stk[MYOS_TASK_STK_START]);
    myos_Start();
    while(1);
}
