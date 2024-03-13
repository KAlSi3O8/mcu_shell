#include "myos_log.h"

void myos_log(char *str) {
    usart1_sendstr("[log] ");
    usart1_sendstr(str);
    usart1_sendstr("\r\n");
    return;
}

void myos_logreg(INT32U addr) {
    char str[24] = {0};
    snprintf(str, 24, "0x%08X:0x%08X\r\n", addr, *((INT32U *)addr));

    usart1_sendstr("[log][reg] ");
    usart1_sendnstr(str, 24);
    usart1_sendstr("\r\n");
    return;
}

void myos_logvar(INT32U var) {
    char str[24] = {0};
    snprintf(str, 24, "%08X(%u)", var, var);

    usart1_sendstr("[log][var] 0x");
    usart1_sendstr(str);
    usart1_sendstr("\r\n");
    return;
}

void myos_logser(void) {
    char str[20] = {0};
    static uint8_t num = 0;
    num++;
    snprintf(str, 20, "Checkpoint %d\r\n", num);
    usart1_sendstr(str);
}