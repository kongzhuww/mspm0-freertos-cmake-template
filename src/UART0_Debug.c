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

// 阻塞发送字符串（仅供栈溢出等致命错误场景使用）
void debug_uart_send_string(char* str)
{
	while (*str != '\0') {
		debug_uart_send_char(*str++);
	}
}



#if defined(__GNUC__)
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "semphr.h"

extern StreamBufferHandle_t uart_stream_buffer;
extern SemaphoreHandle_t logger_mutex;

/*
 * [核心防死锁标志] 追踪 TX 中断链是否正在运行。
 * true  = ISR 正在以 "发送→中断→发送→中断..." 的接力方式排空缓冲区
 * false = ISR 已因缓冲区排空而自行停机，需要外部手动 "起搏" 才能重启
 */
static volatile bool uart_tx_active = false;

/*
 * [原子起搏器] 确保 TX 中断链正在运行。
 * 如果链已停止且缓冲区有数据，则从缓冲区取出第一个字节直接塞给硬件寄存器，
 * 启动整条发送链。整个操作在关中断的临界区内完成，杜绝一切竞态。
 * 必须从任务上下文调用。
 */
static void uart_tx_ensure_draining(void)
{
    taskENTER_CRITICAL();
    if (!uart_tx_active) {
        char c;
        BaseType_t dummy = pdFALSE;
        // 在临界区内使用 FromISR 版本是安全的（中断已关，等价于 ISR 上下文）
        size_t bytes = xStreamBufferReceiveFromISR(uart_stream_buffer, &c, 1, &dummy);
        if (bytes > 0) {
            uart_tx_active = true;
            DL_UART_Main_enableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_TX);
            DL_UART_Main_transmitData(UART_DEBUG_INST, c);
        }
    }
    taskEXIT_CRITICAL();
}

// 适配 GCC 环境的 printf 重定向
int _write(int file, char *ptr, int len)
{
    // [致命防坑] 判断当前是否处于中断上下文 (ISR)。
    // Cortex-M0+ 读取 SCB->ICSR 的 VECTACTIVE 位域，非零说明在 ISR 中。
    // 中断中严禁调用带阻塞属性的 FreeRTOS API，否则秒死锁/HardFault！
    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0) {
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

    // 异步日志引擎已启动：受互斥锁保护，将数据推入环形缓冲区
    if (xSemaphoreTake(logger_mutex, portMAX_DELAY) == pdTRUE) {
        int i;
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                char cr = '\r';
                // [关键] 在每次可能阻塞的 Send 之前，确保 TX 链在跑！
                // 否则缓冲区满时任务阻塞，而 ISR 已停机 → 永久死锁
                uart_tx_ensure_draining();
                xStreamBufferSend(uart_stream_buffer, &cr, 1, portMAX_DELAY);
            }
            uart_tx_ensure_draining();
            xStreamBufferSend(uart_stream_buffer, &ptr[i], 1, portMAX_DELAY);
        }
        xSemaphoreGive(logger_mutex);
    }

    // 最终保底起搏：确保刚推入的最后一批数据也能被排走
    uart_tx_ensure_draining();

    return len;
}

// 串口中断服务函数，专门接管底层的物理发送任务
void UART0_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_DEBUG_INST)) {
        case DL_UART_MAIN_IIDX_TX:
        {
            char c;
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;

            // 如果 StreamBuffer 尚未初始化，直接停机
            if (uart_stream_buffer == NULL) {
                DL_UART_Main_disableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_TX);
                uart_tx_active = false;
                break;
            }

            // 从流缓冲区中非阻塞提取 1 个字符
            size_t bytes = xStreamBufferReceiveFromISR(uart_stream_buffer, &c, 1, &xHigherPriorityTaskWoken);

            if (bytes > 0) {
                // 有数据，推入物理发送寄存器（字节发完后硬件自动触发下一次 TX 中断）
                DL_UART_Main_transmitData(UART_DEBUG_INST, c);
            } else {
                // 缓冲区已空，关闭 TX 中断停机，标记链已死，等待 ensure_draining 来重启
                DL_UART_Main_disableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_TX);
                uart_tx_active = false;
            }

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            break;
        }
        default:
            break;
    }
}
#endif
