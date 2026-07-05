#include "zdt_motor.h"
#include "ti_msp_dl_config.h"

// 使用在 ti_msp_dl_config.h 中定义好的 UART 实例
#include <ti/driverlib/m0p/dl_core.h>
#include <ti/driverlib/driverlib.h>

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

void zdt_motor_init(void)
{
    // 串口硬件已由 SYSCFG_DL_init() 初始化完毕
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
