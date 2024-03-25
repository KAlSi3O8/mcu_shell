#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <stdlib.h>

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
        UART1_SendStr("Usage: devmem reg_addr [w value]\r\n");
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
        UART1_SendStr(tmp);
        free(tmp);
        return 0;
    }
    if(arg_len == 1 && arg[0] == 'w') {
        arg = args;
        UART1_SendStr(arg);
        arg_len = get_token_size(&args);
        if(arg_len == 0) {
            UART1_SendStr("lack of params\r\n");
            UART1_SendStr("Usage: devmem reg_addr [w value]\r\n");
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
        UART1_SendStr(tmp);
        free(tmp);
        *addr = val;
    } else {
        UART1_SendStr("params worng\r\n");
        UART1_SendStr("Usage: devmem reg_addr [w value]\r\n");
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
                UART1_SendStr("mcu shell cmd:\r\n");
                UART1_SendStr("devmem - show value of register\r\n");
                UART1_SendStr("help - show this info\r\n");
            } else {
                UART1_SendStr("Unknown cmd\r\n");
                UART1_SendStr("use \"help\" to show cmd info\r\n");
            }
            break;
        case 6:
            if(strncmp(token, "devmem", token_len) == 0) {
                ret = devmem(cmd);
            } else {
                UART1_SendStr("Unknown cmd\r\n");
                UART1_SendStr("use \"help\" to show cmd info\r\n");
            }
            break;
        default:
            UART1_SendStr("Unknown cmd\r\n");
            UART1_SendStr("use \"help\" to show cmd info\r\n");
            break;
    }

    return ret;
}