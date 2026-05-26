#include "logger.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "semphr.h"
#include "task.h"
#include "ti_msp_dl_config.h"
#include "UART0_Debug.h"

#define STREAM_BUFFER_SIZE  512
#define STREAM_BUFFER_TRIGGER_LEVEL 1
#define LOGGER_TASK_STACK_SIZE 256

/* 全局句柄供 UART0_Debug.c 中的 _write 使用 */
StreamBufferHandle_t uart_stream_buffer = NULL;
SemaphoreHandle_t logger_mutex = NULL;

/* 静态内存分配区 */
static uint8_t ucStreamBufferStorage[STREAM_BUFFER_SIZE];
static StaticStreamBuffer_t xStreamBufferStruct;

static StaticSemaphore_t xMutexBuffer;

static StackType_t logger_task_stack[LOGGER_TASK_STACK_SIZE];
static StaticTask_t logger_task_tcb;

/* 后台发送任务 */
static void vLoggerTask(void *pvParameters)
{
    (void)pvParameters;
    char buf[32];
    size_t received_bytes;
    
    for (;;) {
        // 阻塞等待流缓冲区内出现数据，批量提取以降低 RTOS API 调度开销
        received_bytes = xStreamBufferReceive(uart_stream_buffer, buf, sizeof(buf), portMAX_DELAY);
        if (received_bytes > 0) {
            for (size_t i = 0; i < received_bytes; i++) {
                // 等待硬件空闲并发送物理字节
                while( DL_UART_isBusy(UART_DEBUG_INST) == true );
                DL_UART_Main_transmitData(UART_DEBUG_INST, buf[i]);
            }
        }
    }
}

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
    
    // 3. 启动低优先级的后台发送任务（设为0空闲优先级，完全不抢占业务算力）
    xTaskCreateStatic(
        vLoggerTask,
        "LoggerTask",
        LOGGER_TASK_STACK_SIZE,
        NULL,
        0, // 空闲优先级
        logger_task_stack,
        &logger_task_tcb
    );
}
