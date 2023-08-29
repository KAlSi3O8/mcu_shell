#ifndef __MYDRIVER_UART
#define __MYDRIVER_UART

#define DMA_BUFFER_SIZE 1024

void usart1_init(void);
void usart1_sendstr(char *str);

#endif