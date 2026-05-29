#ifndef UART0_DEBUG_H
#define UART0_DEBUG_H

#include "ti_msp_dl_config.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化调试串口（当前为空，实际配置由 SYSCFG_DL_init 完成）
 */
void debug_uart_init(void);

/** @brief 阻塞发送单个字符（仅供栈溢出等致命错误场景使用） */
void debug_uart_send_char(char ch);

/** @brief 阻塞发送字符串（仅供栈溢出等致命错误场景使用） */
void debug_uart_send_string(char* str);

#ifdef __cplusplus
}
#endif

#endif /* UART0_DEBUG_H */