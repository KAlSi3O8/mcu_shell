#ifndef __MYOS_LOG_H
#define __MYOS_LOG_H

#include <stm32f4xx_conf.h>
#include <stdio.h>
#include "../user/myDriver_uart.h"
#include "myos_cpu.h"

void myos_log(char *str);
void myos_logreg(INT32U addr);
void myos_logvar(INT32U var);
void myos_logser(void);

#endif