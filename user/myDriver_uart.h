#ifndef __MYDRIVER_UART_H
#define __MYDRIVER_UART_H

#define BUFFER_SIZE 256
#define SLOT_SIZE   2
#define SLOT_A      0
#define SLOT_B      1
#define SLOT_A_MSK  0x1
#define SLOT_B_MSK  0x2
#define SLOT_MSK(x) (1 << (x))

struct _uart_dual_buf {
    uint8_t slot_num;
    uint8_t slot_flag;
    uint8_t index;
    uint8_t buf_len[SLOT_SIZE];
    uint8_t buf[SLOT_SIZE][BUFFER_SIZE];
};
extern struct _uart_dual_buf uart1_dual_buf;

void UART1_Init(void);
int UART1_SendStr(char *str);
int UART1_SendnStr(char *str, uint32_t n);
int32_t UART1_GetReadySlot(void);
int32_t UART1_SetSlot(uint8_t slot);

#endif