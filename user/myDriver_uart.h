#ifndef __MYDRIVER_UART
#define __MYDRIVER_UART

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

void usart1_init(void);
void usart1_sendstr(char *str);
void usart1_sendnstr(char *str, uint32_t n);
int32_t usart1_get_ready_slot(void);
int32_t usart1_set_slot(uint8_t slot);

#endif