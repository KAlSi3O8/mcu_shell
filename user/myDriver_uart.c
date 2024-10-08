#include <stm32f4xx_conf.h>
#include <string.h>
#include "myDriver_uart.h"

struct _uart_dual_buf uart1_dual_buf;

/*
 * @brief   re-define _write for printf()
 */
int _write(int fd, char *ptr, int len) {
    return usart1_sendnstr(ptr, len);
}

/**
 * @brief   set current usart1 buf slot
 * @param   slot: slot num(SLOT_A， SLOT_B)
 * @retval  -1: param slot is not a slot num
 * @retval  -2: slot has not been read, data may overwrite
 * @retval  0: set slot success
*/
int32_t usart1_set_slot(uint8_t slot) {
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
int32_t usart1_switch_slot(void) {
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
int32_t usart1_get_ready_slot(void) {
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
void usart1_init(void) {
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
    hNVIC.NVIC_IRQChannelSubPriority = 14;
    NVIC_Init(&hNVIC);

// Initing RX buf
    memset(&uart1_dual_buf, 0, sizeof(uart1_dual_buf));

    USART_Cmd(USART1, ENABLE);
    return;
}

/**
 * @brief   send str to usart1
 * @param   str: message to be sent
 * @retval  None
 */
int usart1_sendstr(char *str) {
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
 * @retval  number of bytes have sent
 */
int usart1_sendnstr(char *str, int n) {
    for(int i = 0; i < n; i++) {
        USART_SendData(USART1, str[i]);
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
    return n;
}

void USART1_IRQHandler(void) {
    char tmp;
    if(USART_GetITStatus(USART1, USART_IT_RXNE)) {
        tmp = USART_ReceiveData(USART1);
        USART_SendData(USART1, tmp);
        while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
        if(tmp == '\b') {
            USART_SendData(USART1, ' ');
            while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
            USART_SendData(USART1, '\b');
            while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
            uart1_dual_buf.index--;
            uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = 0;
        } else {
            uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = tmp;
            uart1_dual_buf.index++;
        }
        if(tmp == '\r') {
            USART_SendData(USART1, '\n');
            while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
            uart1_dual_buf.buf[uart1_dual_buf.slot_num][uart1_dual_buf.index] = 0;
            usart1_switch_slot();
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
    return;
}