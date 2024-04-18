#include <stm32f4xx_conf.h>
#include <string.h>
#include <stdio.h>
#include "myDriver_uart.h"

struct _uart_dual_buf uart1_dual_buf;

/*
 * re-define _write for printf()
 */
int _write(int fd, char *ptr, int len) {
    return UART1_SendnStr(ptr, len);
}

/**
 * @brief   set current usart1 buf slot
 * @param   slot: slot num(SLOT_A， SLOT_B)
 * @retval  -1: param slot is not a slot num
 * @retval  -2: slot has not been read, data may overwrite
 * @retval  0: set slot success
*/
int32_t UART1_SetSlot(uint8_t slot) {
    if(slot != SLOT_A && SLOT_B != 1) {
        return -1;
    }
    if(uart1_dual_buf.slot_flag & SLOT_MSK(slot)) {
        return -2;
    }
    uart1_dual_buf.slot_num = slot;
    return 0;
}

/**
 * @brief   switch usart1 RX buffer slot
 * @retval  -1: slot has not been read, data may overwrite
 * @retval  0: switch slot success
*/
int32_t UART1_SwitchSlot(void) {
    // set slot ready to read flag
    uart1_dual_buf.slot_flag |= SLOT_MSK(uart1_dual_buf.slot_num);
    // save data len of current slot
    uart1_dual_buf.buf_len[uart1_dual_buf.slot_num] = uart1_dual_buf.index;
    // switch slot num
    uart1_dual_buf.slot_num ^= 0x01;
    // reset buf index
    uart1_dual_buf.index = 0;
    // check slot status
    if(uart1_dual_buf.slot_flag & SLOT_MSK(uart1_dual_buf.slot_num)) {
        return -1;
    }
    return 0;
}

/**
 * @brief   get usart1 ready buffer num
 * @retval  -1: no buffer is ready
 * @retval  0: ret buffer is ready to read
*/
int32_t UART1_GetReadySlot(void) {
    uint8_t ready_slot;
    ready_slot = uart1_dual_buf.slot_num ? SLOT_A : SLOT_B;
    if((uart1_dual_buf.slot_flag & SLOT_MSK(ready_slot)) == 0) {
        return -1;
    }
    return ready_slot;
}

/**
 * @brief   init usart1 clock/gpio/interrup and enable usart1
*/
void UART1_Init(void) {
    GPIO_InitTypeDef hGPIOA;
    USART_InitTypeDef hUSART1;
    NVIC_InitTypeDef hNVIC;

// Enable Clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);

// Config GPIO PA9-TX PA10-RX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    hGPIOA.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    hGPIOA.GPIO_Mode = GPIO_Mode_AF;
    hGPIOA.GPIO_Speed = GPIO_Speed_50MHz;
    hGPIOA.GPIO_OType = GPIO_OType_PP;
    hGPIOA.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &hGPIOA);

// Config Uart
    USART_StructInit(&hUSART1);
    hUSART1.USART_BaudRate = 115200;
    USART_Init(USART1, &hUSART1);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

// Config UART RX NVIC
    hNVIC.NVIC_IRQChannel = USART1_IRQn;
    hNVIC.NVIC_IRQChannelCmd = ENABLE;
    hNVIC.NVIC_IRQChannelPreemptionPriority = 0;
    hNVIC.NVIC_IRQChannelSubPriority = 10;
    NVIC_Init(&hNVIC);

// Initing RX buf
    memset(&uart1_dual_buf, 0, sizeof(uart1_dual_buf));

    USART_Cmd(USART1, ENABLE);
    return;
}

/**
 * @brief   send str to usart1
 * @param   str: message to be sent
 * @retval  total bytes have sent
 */
int UART1_SendStr(char *str) {
    int len = strlen(str);
    for(int i = 0; i < len; i++) {
        USART_SendData(USART1, str[i]);
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
    return len;
}

/**
 * @brief   send n bytes of str to usart1
 * @param   str: message to be sent
 * @param   n: number of bytes to send
 * @retval  total bytes have sent
 */
int  UART1_SendnStr(char *str, uint32_t n) {
    for(int i = 0; i < n; i++) {
        USART_SendData(USART1, str[i]);
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
    return n;
}

void USART1_IRQHandler(void) {
    char tmp;
    static uint8_t status = 0;
    if(USART_GetITStatus(USART1, USART_IT_RXNE)) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        tmp = USART_ReceiveData(USART1);
        switch (status) {
        case 0x00:
            switch (tmp) {
            case '\b':
                UART1_SendStr("\b \b");
                uart1_dual_buf.index--;
                uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = 0;
                break;
            case '\r':
                UART1_SendStr("\r\n");
                uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = 0;
                UART1_SwitchSlot();
                break;
            case 0x1b:
                status |= 0x01;
                break;
            default:
                USART_SendData(USART1, tmp);
                uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = tmp;
                uart1_dual_buf.index++;
                break;
            }
            break;
        case 0x01:
            switch (tmp) {
            case '[':
                status |= 0x02;
                break;
            default:
                status = 0;
                USART_SendData(USART1, tmp);
                uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = tmp;
                uart1_dual_buf.index++;
                break;
            }
            break;
        case 0x03:
            switch (tmp) {
            case 'A':
            case 'B':
                UART1_SendStr("\r> ");
                for (int i = uart1_dual_buf.index; i > 0; i--) {
                    USART_SendData(USART1, ' ');
                    while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
                }
                UART1_SendStr("\r\x1b[2C");
                uart1_dual_buf.buf_len[uart1_dual_buf.slot_num] = uart1_dual_buf.index;
                UART1_SetSlot(uart1_dual_buf.slot_num ^ 0x01);
                uart1_dual_buf.index = uart1_dual_buf.buf_len[uart1_dual_buf.slot_num];
                UART1_SendnStr(uart1_dual_buf.buf[uart1_dual_buf.slot_num], uart1_dual_buf.buf_len[uart1_dual_buf.slot_num]);
                status = 0;
                break;
            default:
                status = 0;
                break;
            }
            break;
        }
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
    return;
}