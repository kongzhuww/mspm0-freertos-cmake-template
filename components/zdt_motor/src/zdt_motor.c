#include "zdt_motor.h"
#include "ti_msp_dl_config.h"

// 使用在 ti_msp_dl_config.h 中定义好的 UART 实例
#include <ti/driverlib/m0p/dl_core.h>
#include <ti/driverlib/driverlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "logger.h"

// 默认校验字节（出厂默认 0x6B，可在电机菜单 Checksum 中修改）
#define ZDT_CHECKSUM    0x6B

/**
 * @brief 通过串口发送一个字节
 */
static void zdt_uart_send_byte(uint8_t data)
{
    // 先等待上一个字节发送完毕，再写入新数据（与 Debug UART 保持一致的正确顺序）
    while (DL_UART_isBusy(UART_ZDT_INST));
    DL_UART_Main_transmitData(UART_ZDT_INST, data);
}

/**
 * @brief 发送多字节数组
 */
static void zdt_uart_send_array(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        zdt_uart_send_byte(data[i]);
    }
}

// ----------------- 电机 RX 回执处理 -----------------
#define ZDT_RX_QUEUE_LEN    128
static StaticQueue_t zdt_rx_queue_tcb;
static uint8_t zdt_rx_queue_storage[ZDT_RX_QUEUE_LEN * sizeof(uint8_t)];
static QueueHandle_t zdt_rx_queue = NULL;

#define ZDT_RX_TASK_STACK_SIZE    256
static StaticTask_t zdt_rx_task_tcb;
static StackType_t zdt_rx_task_stack[ZDT_RX_TASK_STACK_SIZE];

static void vZdtRxTask(void *pvParameters)
{
    uint8_t rx_byte;
    uint8_t frame_buf[16];
    uint8_t frame_idx = 0;

    for (;;) {
        // 持续等待电机的返回字节
        if (xQueueReceive(zdt_rx_queue, &rx_byte, portMAX_DELAY) == pdTRUE) {
            frame_buf[frame_idx++] = rx_byte;
            
            // 简单的帧定界（依赖 0x6B 结尾）
            if (rx_byte == ZDT_CHECKSUM) {
                // 如果是一般应答帧，长度大多是 4 字节（地址 + 功能码 + 状态 + 0x6B）
                if (frame_idx == 4) {
                    LOG_INFO("[ZDT] Motor ACK -> Addr: %02X, Cmd: %02X, Status: %02X", 
                             frame_buf[0], frame_buf[1], frame_buf[2]);
                    
                    if (frame_buf[2] == 0x02) {
                        // 0x02 表示成功接收或执行完毕
                    } else if (frame_buf[2] == 0xE2) {
                        LOG_ERROR("[ZDT] Motor Error/Stalled! Status: %02X", frame_buf[2]);
                    } else if (frame_buf[2] == 0xEE) {
                        LOG_ERROR("[ZDT] Motor Command Error! Status: %02X", frame_buf[2]);
                    }
                }
                // 处理完毕，清空缓冲准备下一帧
                frame_idx = 0;
            }
            
            // 防溢出
            if (frame_idx >= sizeof(frame_buf)) {
                frame_idx = 0;
            }
        }
    }
}

// ----------------- UART2 中断服务函数 -----------------
void UART2_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_ZDT_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            // 读取接收到的数据
            while (DL_UART_Main_isRXFIFOEmpty(UART_ZDT_INST) == false) {
                uint8_t data = DL_UART_Main_receiveData(UART_ZDT_INST);
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                if (zdt_rx_queue != NULL) {
                    xQueueSendFromISR(zdt_rx_queue, &data, &xHigherPriorityTaskWoken);
                }
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            break;
        default:
            break;
    }
}

void zdt_motor_init(void)
{
    // 串口硬件已由 SYSCFG_DL_init() 初始化完毕
    
    // 1. 创建 RX 队列
    zdt_rx_queue = xQueueCreateStatic(ZDT_RX_QUEUE_LEN, sizeof(uint8_t), zdt_rx_queue_storage, &zdt_rx_queue_tcb);
    
    // 2. 创建解析任务
    xTaskCreateStatic(
        vZdtRxTask,
        "ZdtRxTask",
        ZDT_RX_TASK_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        zdt_rx_task_stack,
        &zdt_rx_task_tcb
    );
    
    // 3. 使能 UART_ZDT RX 中断
    DL_UART_Main_enableInterrupt(UART_ZDT_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_SetPriority(UART_ZDT_INST_INT_IRQN, 3); // 在配置里其实已经设置了，安全起见重申
    NVIC_EnableIRQ(UART_ZDT_INST_INT_IRQN);
}

void zdt_motor_enable(uint8_t addr, bool enable)
{
    // 使能指令格式：地址 + 0xF3 + 0xAB + 使能状态 + 多机同步标志 + 校验字节
    // 示例（使能）：01 F3 AB 01 00 6B
    // 示例（失能）：01 F3 AB 00 00 6B
    uint8_t cmd[6];
    cmd[0] = addr;
    cmd[1] = 0xF3;
    cmd[2] = 0xAB;
    cmd[3] = enable ? 0x01 : 0x00;
    cmd[4] = 0x00;          // 多机同步标志：不启用
    cmd[5] = ZDT_CHECKSUM;

    zdt_uart_send_array(cmd, 6);
}

void zdt_motor_set_speed(uint8_t addr, uint8_t dir, uint16_t rpm, uint8_t acc)
{
    // 速度模式指令格式：地址 + 0xF6 + 方向 + 速度(2字节) + 加速度 + 多机同步标志 + 校验字节
    // 示例：01 F6 01 05 DC 0A 00 6B
    // 方向：00=CW(顺时针), 01=CCW(逆时针)
    uint8_t cmd[8];
    cmd[0] = addr;
    cmd[1] = 0xF6;
    cmd[2] = dir;                   // 方向
    cmd[3] = (rpm >> 8) & 0xFF;     // 速度高字节
    cmd[4] = rpm & 0xFF;            // 速度低字节
    cmd[5] = acc;                   // 加速度档位
    cmd[6] = 0x00;                  // 多机同步标志：不启用
    cmd[7] = ZDT_CHECKSUM;

    zdt_uart_send_array(cmd, 8);
}

void zdt_motor_move_rel(uint8_t addr, uint8_t dir, uint16_t speed, uint8_t acc, uint32_t pulses)
{
    // 位置模式指令格式：地址 + 0xFD + 方向 + 速度(2字节) + 加速度 + 脉冲数(4字节) + 相对/绝对标志 + 多机同步标志 + 校验字节
    // 示例：01 FD 01 05 DC 00 00 00 7D 00 00 00 6B
    // 方向：00=CW, 01=CCW
    // 相对/绝对：00=相对位置, 01=绝对位置
    uint8_t cmd[13];
    cmd[0]  = addr;
    cmd[1]  = 0xFD;
    cmd[2]  = dir;                      // 方向
    cmd[3]  = (speed >> 8) & 0xFF;      // 速度高字节
    cmd[4]  = speed & 0xFF;             // 速度低字节
    cmd[5]  = acc;                      // 加速度档位
    cmd[6]  = (pulses >> 24) & 0xFF;    // 脉冲数高字节
    cmd[7]  = (pulses >> 16) & 0xFF;
    cmd[8]  = (pulses >> 8)  & 0xFF;
    cmd[9]  = pulses & 0xFF;            // 脉冲数低字节
    cmd[10] = 0x00;                     // 相对位置模式
    cmd[11] = 0x00;                     // 多机同步标志：不启用
    cmd[12] = ZDT_CHECKSUM;

    zdt_uart_send_array(cmd, 13);
}

void zdt_motor_move_angle_abs(uint8_t addr, uint8_t dir, uint16_t speed, uint8_t acc, float angle)
{
    // 绝对位置模式指令格式：地址 + 0xFD + 方向 + 速度(2字节) + 加速度 + 脉冲数(4字节) + 相对/绝对标志 + 多机同步标志 + 校验字节
    // 绝对标志：01=绝对位置
    // 假设 16 细分下，3200 脉冲 = 360 度
    uint32_t pulses = (uint32_t)((angle / 360.0f) * 3200.0f);
    
    uint8_t cmd[13];
    cmd[0]  = addr;
    cmd[1]  = 0xFD;
    cmd[2]  = dir;                      // 方向（在绝对模式中通常会被底层覆盖，但按要求传0/1）
    cmd[3]  = (speed >> 8) & 0xFF;      // 速度高字节
    cmd[4]  = speed & 0xFF;             // 速度低字节
    cmd[5]  = acc;                      // 加速度档位
    cmd[6]  = (pulses >> 24) & 0xFF;    // 脉冲数高字节
    cmd[7]  = (pulses >> 16) & 0xFF;
    cmd[8]  = (pulses >> 8)  & 0xFF;
    cmd[9]  = pulses & 0xFF;            // 脉冲数低字节
    cmd[10] = 0x01;                     // 0x01 = 绝对位置模式
    cmd[11] = 0x00;                     // 多机同步标志：不启用
    cmd[12] = ZDT_CHECKSUM;

    zdt_uart_send_array(cmd, 13);
}

void zdt_motor_stop(uint8_t addr)
{
    // 立即停止指令格式：地址 + 0xFE + 0x98 + 多机同步标志 + 校验字节
    // 示例：01 FE 98 00 6B
    uint8_t cmd[5];
    cmd[0] = addr;
    cmd[1] = 0xFE;
    cmd[2] = 0x98;
    cmd[3] = 0x00;          // 多机同步标志：不启用
    cmd[4] = ZDT_CHECKSUM;

    zdt_uart_send_array(cmd, 5);
}
