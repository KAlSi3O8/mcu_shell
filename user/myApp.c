#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myDriver_uart.h"
#include "myDriver_sccb.h"
#include "myOV7670.h"

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

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);

    return;
}


int get_token_size(char** str) {
    int size = 0;
    while(*(*str) != ' ' && *(*str) != '\0') {
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
        UART1_SendStr("Usage: devmem reg_addr [w value]\r\n");
        return 1;
    }
    addr = strtoul(arg, NULL, 0);

    arg = args;
    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        printf("Value in 0x%08x is 0x%08x(%u)\r\n", addr, *addr, *addr);
        return 0;
    }
    if(arg_len == 1 && arg[0] == 'w') {
        arg = args;
        arg_len = get_token_size(&args);
        if(arg_len == 0) {
            UART1_SendStr("lack of params\r\n");
            UART1_SendStr("Usage: devmem reg_addr [w value]\r\n");
            return -1;
        }

        val = strtoul(arg, NULL, 0);
        *addr = val;
        printf("Wrote 0x%08x(%u) to 0x%08x\r\n", val, val, addr);
    } else {
        UART1_SendStr("params worng\r\n");
        UART1_SendStr("Usage: devmem reg_addr [w value]\r\n");
        return -1;
    }

    return 0;
}

int sccb(char* args) {
    int arg_len;
    char *arg = args;
    uint8_t addr;
    uint8_t value;

    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        UART1_SendStr("Usage: sccb addr [w value]\r\n");
        return 1;
    }

    addr = strtoul(arg, NULL, 0);
    arg = args;
    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        value = SCCB_ReadReg(OV7670_ADDR, addr);
        printf("Value at 0x%02x is 0x%02x(%u)\r\n", addr, value, value);
    } else if (arg_len == 1 && arg[0] == 'w') {
        arg = args;
        arg_len = get_token_size(&args);
        if (arg_len != 0) {
            value = strtoul(arg, NULL, 0);
            SCCB_WriteReg(OV7670_ADDR, addr, value);
            printf("Wrote 0x%02x(%u) to 0x%02x\r\n", value, value, addr);
        } else {
            UART1_SendStr("lack of params\r\n");
            UART1_SendStr("Usage: sccb addr [w value]\r\n");
        }
    } else {
        UART1_SendStr("params worng\r\n");
        UART1_SendStr("Usage: sccb addr [w value]\r\n");
        return -1;
    }

    return 0;
}

int setThreshold(char* args) {
    int arg_len;
    char *arg = args;

    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        UART1_SendStr("Usage: threshold value(0x00~0xFF)\r\n");
        return 1;
    }

    grayThreshold = strtol(arg, NULL, 0);
    printf("Gray threshold is set to %d\r\n", grayThreshold);
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
                UART1_SendStr("mcu shell cmd:\r\n");
                UART1_SendStr("sccb - read/write value of camera register\r\n");
                UART1_SendStr("devmem - read/write value of register\r\n");
                UART1_SendStr("getexp - get camera exposure time\r\n");
                UART1_SendStr("capture - capture one frame from camera\r\n");
                UART1_SendStr("threshold - set gray threshold value\r\n");
                UART1_SendStr("help - show this info\r\n");
            } else if (strncmp(token, "sccb", token_len) == 0) {
                ret = sccb(cmd);
            } else {
                goto wrong;
            }
            break;
        case 6:
            if(strncmp(token, "devmem", token_len) == 0) {
                ret = devmem(cmd);
            } else if(strncmp(token, "getexp", token_len) == 0) {
                printf("AEC = %04x\r\n", OV7670_GetAEC());
            } else {
                goto wrong;
            }
            break;
        case 7:
            if(strncmp(token, "capture", token_len) == 0) {
                DCMI_CaptureCmd(ENABLE);
            } else {
                goto wrong;
            }
            break;
        case 9:
            if(strncmp(token, "threshold", token_len) == 0) {
                ret = setThreshold(cmd);
            } else {
                goto wrong;
            }
            break;
        default:
            wrong:
            printf("Unknown cmd %s(%d)\r\n", token, token_len);
            UART1_SendStr("use \"help\" to show cmd info\r\n");
            break;
    }

    UART1_SendStr("> ");

    return ret;
}