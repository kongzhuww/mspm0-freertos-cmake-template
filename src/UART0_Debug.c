#include "UART0_Debug.h"

// 初始化调试串口
void debug_uart_init(void)
{
	// 发送采用 RTOS 后台轮询或由硬件机制接管，此处不再开启 RX/TX 底层中断注册
}

//发送单个字符
void debug_uart_send_char(char ch)
{
	while( DL_UART_isBusy(UART_DEBUG_INST) == true );
	DL_UART_Main_transmitData(UART_DEBUG_INST, ch);
}

//发送字符串
void debug_uart_send_string(char* str)
{
	while(*str!=0&&str!=0)
	{
		debug_uart_send_char(*str++);
	}
}



#if defined(__GNUC__)
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "semphr.h"

extern StreamBufferHandle_t uart_stream_buffer;
extern SemaphoreHandle_t logger_mutex;

// 适配 GCC 环境的 printf 重定向
int _write(int file, char *ptr, int len)
{
    // [致命防坑] 判断当前是否处于中断上下文 (ISR)。
    // 在 Cortex-M0+ 中，读取 SCB->ICSR (Interrupt Control and State Register) 寄存器
    // 的最低 6 位 (VECTACTIVE) 即可判断。如果非零，说明正在中断处理函数中。
    // 中断中严禁调用带阻塞属性的 FreeRTOS API，否则秒死锁/HardFault！
    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0) {
        // 如果实在想看中断里的打印，可以放开这里的底层硬件轮询发送。
        // 但为了保证中断不被拖慢，直接丢弃是最安全的做法。
        return len;
    }

    // 如果异步日志引擎还未初始化，回退到原生的硬件阻塞发送
    if (uart_stream_buffer == NULL || logger_mutex == NULL) {
        int i;
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                while( DL_UART_isBusy(UART_DEBUG_INST) == true );
                DL_UART_Main_transmitData(UART_DEBUG_INST, '\r');
            }
            while( DL_UART_isBusy(UART_DEBUG_INST) == true );
            DL_UART_Main_transmitData(UART_DEBUG_INST, ptr[i]);
        }
        return len;
    }

    // 异步日志引擎已启动：受互斥锁保护，将数据直接推入环形内存，极其快速
    if (xSemaphoreTake(logger_mutex, portMAX_DELAY) == pdTRUE) {
        int i;
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                char cr = '\r';
                xStreamBufferSend(uart_stream_buffer, &cr, 1, portMAX_DELAY);
            }
            xStreamBufferSend(uart_stream_buffer, &ptr[i], 1, portMAX_DELAY);
        }
        xSemaphoreGive(logger_mutex);
    }
    
    return len;
}
#endif
