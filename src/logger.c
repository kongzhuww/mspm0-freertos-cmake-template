#include "logger.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "semphr.h"
#include "ti_msp_dl_config.h"
#include "UART0_Debug.h"

#define STREAM_BUFFER_SIZE  512
#define STREAM_BUFFER_TRIGGER_LEVEL 1

/* 全局句柄供 UART0_Debug.c 中的 _write 使用 */
StreamBufferHandle_t uart_stream_buffer = NULL;
SemaphoreHandle_t logger_mutex = NULL;

/* 静态内存分配区 */
static uint8_t ucStreamBufferStorage[STREAM_BUFFER_SIZE];
static StaticStreamBuffer_t xStreamBufferStruct;

static StaticSemaphore_t xMutexBuffer;

void logger_init(void)
{
    // 1. 创建流缓冲区
    uart_stream_buffer = xStreamBufferCreateStatic(
                            STREAM_BUFFER_SIZE,
                            STREAM_BUFFER_TRIGGER_LEVEL,
                            ucStreamBufferStorage,
                            &xStreamBufferStruct);
                            
    // 2. 创建互斥锁，保障多任务 printf 调用不乱码
    logger_mutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    
    // 3. 确保 UART0 的中断在 NVIC 中被使能，以支持完全中断驱动的日志发送
    NVIC_EnableIRQ(UART_DEBUG_INST_INT_IRQN);
}
