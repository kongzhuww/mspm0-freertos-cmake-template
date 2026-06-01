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
void debug_uart_send_string(const char* str)
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
 * 如果链已停止且缓冲区有数据，则直接使能 TX 中断启动整条发送链。
 * 整个操作在关中断的临界区内完成，杜绝一切竞态。
 * 必须从任务上下文调用。
 */
static void uart_tx_ensure_draining(void)
{
    taskENTER_CRITICAL();
    if (!uart_tx_active) {
        if (uart_stream_buffer != NULL && !xStreamBufferIsEmpty(uart_stream_buffer)) {
            uart_tx_active = true;
            DL_UART_Main_enableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_TX);
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

    // 如果异步日志引擎还未初始化，或者调度器未运行/被挂起，或者当前处于临界区（关闭了全局中断），回退到原生的硬件阻塞发送
    if (uart_stream_buffer == NULL || logger_mutex == NULL || 
        xTaskGetSchedulerState() != taskSCHEDULER_RUNNING || 
        __get_PRIMASK() != 0) {
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
        // 使用临时缓冲区进行 \n → \r\n 展开，减少逐字节临界区进出的开销
        char expand_buf[128];
        int buf_pos = 0;
        int i;

        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                expand_buf[buf_pos++] = '\r';
            }
            expand_buf[buf_pos++] = ptr[i];

            // 缓冲区接近满时（预留 2 字节给可能展开的 \r\n，保障边界安全），批量刷入 StreamBuffer
            if (buf_pos >= (int)(sizeof(expand_buf) - 2)) {
                uart_tx_ensure_draining();
                size_t sent = 0;
                int retries = 5;
                while (sent < (size_t)buf_pos) {
                    // 使用短超时代替 portMAX_DELAY，避免 ISR 停机导致的永久阻塞
                    size_t n = xStreamBufferSend(uart_stream_buffer, expand_buf + sent,
                                                buf_pos - sent, pdMS_TO_TICKS(10));
                    if (n == 0) {
                        if (--retries <= 0) {
                            break; // 连续超时失败，丢弃未发送数据，防挂死
                        }
                        // 强制断链自愈：重置 active 状态以确保能正常拉起发送链
                        taskENTER_CRITICAL();
                        uart_tx_active = false;
                        taskEXIT_CRITICAL();
                        uart_tx_ensure_draining(); // ISR 可能已停机，重新起搏
                    } else {
                        sent += n;
                        retries = 5; // 成功写入，重置重试计数
                        uart_tx_ensure_draining(); // 起搏发送链，避免因停机而出现 10ms 卡顿
                    }
                }
                buf_pos = 0;
            }
        }

        // 发送剩余数据
        if (buf_pos > 0) {
            uart_tx_ensure_draining();
            size_t sent = 0;
            int retries = 5;
            while (sent < (size_t)buf_pos) {
                size_t n = xStreamBufferSend(uart_stream_buffer, expand_buf + sent,
                                            buf_pos - sent, pdMS_TO_TICKS(10));
                if (n == 0) {
                    if (--retries <= 0) {
                        break; // 连续超时失败，丢弃数据
                    }
                    // 强制断链自愈：重置 active 状态以确保能正常拉起发送链
                    taskENTER_CRITICAL();
                    uart_tx_active = false;
                    taskEXIT_CRITICAL();
                    uart_tx_ensure_draining();
                } else {
                    sent += n;
                    retries = 5;
                    uart_tx_ensure_draining(); // 起搏发送链，保证连续发送的实时性
                }
            }
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
            bool data_sent = false;

            // 如果 StreamBuffer 尚未初始化，直接停机
            if (uart_stream_buffer == NULL) {
                DL_UART_Main_disableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_TX);
                uart_tx_active = false;
                break;
            }

            // 只要 TX FIFO 未满且环形缓冲区有数据，就持续批量提取写入
            while (!DL_UART_Main_isTXFIFOFull(UART_DEBUG_INST)) {
                size_t bytes = xStreamBufferReceiveFromISR(uart_stream_buffer, &c, 1, &xHigherPriorityTaskWoken);
                if (bytes > 0) {
                    DL_UART_Main_transmitData(UART_DEBUG_INST, c);
                    data_sent = true;
                } else {
                    break;
                }
            }

            if (!data_sent) {
                // 缓冲区已空，没有发送任何数据，关闭 TX 中断停机，标记链已死，等待 ensure_draining 重启
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

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, void *st)
{
    (void)file;
    (void)st;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}
#endif
