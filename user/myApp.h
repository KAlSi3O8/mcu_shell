#ifndef __MYAPP_H
#define __MYAPP_H

void system_start(void);
int get_token_size(char** str);
int devmem(char* args);
int process_cmd(char* cmd, int cmd_len);

#define D1_ON()   GPIO_ResetBits(GPIOF, GPIO_Pin_9)
#define D1_OFF()  GPIO_SetBits(GPIOF, GPIO_Pin_9)

#endif