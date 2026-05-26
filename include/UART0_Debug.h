#ifndef UART0_DEBUG_H
#define UART0_DEBUG_H

#include "ti_msp_dl_config.h"
#include "stdio.h"

void debug_uart_init(void);
void debug_uart_send_char(char ch);
void debug_uart_send_string(char* str);


#endif