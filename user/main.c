#include <stm32f4xx_conf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myos.h"
#include "myos_log.h"
#include "myDriver_uart.h"

#define LED_PORT GPIOF
#define LED_PIN  GPIO_Pin_9 | GPIO_Pin_10
#define LED_CLK  RCC_AHB1ENR_GPIOFEN

#define DMA_DESC_AMOUNT 128
ETH_DMADESCTypeDef DMA_TX_DESC[DMA_DESC_AMOUNT];
ETH_DMADESCTypeDef DMA_RX_DESC[DMA_DESC_AMOUNT];
size_t DMA_DESC_SIZE = sizeof(ETH_DMADESCTypeDef * DMA_DESC_AMOUNT);

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

/*
 * get the len of current token
 * the pointer will move to the beginning of next token
 * please save current token to a net pointer
 */
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

int mdio(char *args) {
    int ret = 0;
    int arg_len;
    char *arg = args;
    uint16_t reg;
    uint16_t value;

    arg_len = get_token_size(&args);
    if(arg_len == 0) {
        printf("Usage: mdio reg [w value]\r\n");
        return 0;
    }
    reg = (uint16_t)strtoul(arg, NULL, 0);
    value = ETH_ReadPHYRegister(0x1, reg);

    arg = args;
    arg_len = get_token_size(&args);
    if(arg_len != 0) {
        if(0 == strncmp(arg, "w", arg_len)) {
            arg = args;
            arg_len = get_token_size(&args);
            value = (uint16_t)strtoul(arg, NULL, 0);
            ret = ETH_WritePHYRegister(0x1, reg, value);
            printf("Write %04x to %04x\r\n", value, reg);
        } else {
            printf("wrong argument!\r\n Usage: mdio reg [w value]\r\n");
            return -1;
        }
    } else {
        printf("The value of %04x is %04x(%d)\r\n", reg, value, value);
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
            } else if(strncmp(token, "mdio", token_len) == 0) {
                ret = mdio(cmd);
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

myos_STK LED1_Stk[MYOS_TASK_STK_SIZE];
void LED1_task(void *arg) {
    while(1) {
        GPIO_SetBits(LED_PORT, GPIO_Pin_9);
        myos_TimeDly(700);
        GPIO_ResetBits(LED_PORT, GPIO_Pin_9);
        myos_TimeDly(900);
    }
}

myos_STK LED2_Stk[MYOS_TASK_STK_SIZE];
void LED2_task(void *arg) {
    while(1) {
        GPIO_SetBits(LED_PORT, GPIO_Pin_10);
        myos_TimeDly(100);
        GPIO_ResetBits(LED_PORT, GPIO_Pin_10);
        myos_TimeDly(200);
        GPIO_SetBits(LED_PORT, GPIO_Pin_10);
        myos_TimeDly(100);
        GPIO_ResetBits(LED_PORT, GPIO_Pin_10);
        myos_TimeDly(1300);
    }
}

myos_STK MCU_Shell_Stk[MYOS_TASK_STK_SIZE];
void MCU_Shell_task(void *arg) {
    int ready_slot;
    while(1) {
        ready_slot = usart1_get_ready_slot();
        if(ready_slot != -1) {
            process_cmd(uart1_dual_buf.buf[ready_slot], uart1_dual_buf.buf_len[ready_slot]);
            uart1_dual_buf.slot_flag &= ~SLOT_MSK(ready_slot);
        }
    }
}

/*
 * TXD0 --> PB12
 * TXD1 --> PB13
 * TXEN --> PB11
 * RXD0 --> PC4
 * RXD1 --> PC5
 * MDC  --> PC1
 * MDIO --> PA2
 * REFC --> PA1
 * CRSD --> PA7
 */
void MAC_Init(void) {
    GPIO_InitTypeDef hETH_GPIO;
    ETH_InitTypeDef hETH;
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_ETHMACEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_ETHMACTXEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_ETHMACRXEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOBEN, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN, ENABLE);

    hETH_GPIO.GPIO_Mode = GPIO_Mode_AF;
    hETH_GPIO.GPIO_OType = GPIO_OType_PP;
    hETH_GPIO.GPIO_PuPd = GPIO_PuPd_NOPULL;
    hETH_GPIO.GPIO_Speed = GPIO_High_Speed;

    hETH_GPIO.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &hETH_GPIO);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

    hETH_GPIO.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOB, &hETH_GPIO);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);

    hETH_GPIO.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOC, &hETH_GPIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

    RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);
    SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

    ETH_StructInit(&hETH);
    // Disable Auto Negotiation
    hETH.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
    hETH.ETH_Speed = ETH_Speed_100M;
    hETH.ETH_Mode = ETH_Mode_FullDuplex;

    if(0 == ETH_Init(&hETH, 1)) {
        printf("ETH_Init Error\r\n");
    }

    // MAC descriptor init
    memset(DMA_TX_DESC, 0, DMA_DESC_SIZE);
    memset(DMA_RX_DESC, 0, DMA_DESC_SIZE);
    
    ETH_MACAddressConfig(ETH_MAC_Address0, "\xaa\xbb\xcc\xdd\xaa\x11");
    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
    ETH_Start();

    printf("ID1:%04x\r\n", ETH_ReadPHYRegister(0x1, 2));
    printf("ID2:%04x\r\n", ETH_ReadPHYRegister(0x1, 3));
}

int main(void) {
    system_start();

    printf("my MAC init Start -->\r\n");
    MAC_Init();
    printf("my MAC init End <--\r\n");

    MCU_Shell_task((void *)0);

    // while(1) {
    //     printf("ID1:%04x\r\n", ETH_ReadPHYRegister(0x1, 2));
    // }
// Enter OS
    myos_Init();
    myos_TaskCreate(&LED1_task, (void *)0, &LED1_Stk[MYOS_TASK_STK_START]);
    myos_TaskCreate(&LED2_task, (void *)0, &LED2_Stk[MYOS_TASK_STK_START]);
    myos_TaskCreate(&MCU_Shell_task, (void *)0, &MCU_Shell_Stk[MYOS_TASK_STK_START]);
    myos_Start();
    while(1);
}
