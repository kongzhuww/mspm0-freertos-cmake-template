#ifndef ZDT_MOTOR_H
#define ZDT_MOTOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 张大头 ZDT X42 / Emm42 V5.0 闭环步进电机 UART 驱动
 *        协议参考：Emm_V5.0步进闭环驱动说明书Rev1.3
 *        默认校验方式：0x6B（出厂默认）
 */

/**
 * @brief 初始化电机驱动（串口硬件已由系统初始化）
 */
void zdt_motor_init(void);

/**
 * @brief 电机使能控制
 *        指令：addr + F3 + AB + 使能状态 + 00 + 6B
 * @param addr 电机地址（默认 1）
 * @param enable true=使能(锁轴), false=失能(松开)
 */
void zdt_motor_enable(uint8_t addr, bool enable);

/**
 * @brief 速度模式控制（电机持续转动直到发停止命令）
 *        指令：addr + F6 + 方向 + 速度H + 速度L + 加速度 + 00 + 6B
 * @param addr 电机地址
 * @param dir 0=CW(顺时针), 1=CCW(逆时针)
 * @param rpm 转速 (RPM)
 * @param acc 加速度档位 (0=不加减速, 1-255 越小加速越慢)
 */
void zdt_motor_set_speed(uint8_t addr, uint8_t dir, uint16_t rpm, uint8_t acc);

/**
 * @brief 位置模式控制（相对位置，转到指定脉冲数后自动停止）
 *        指令：addr + FD + 方向 + 速度H + 速度L + 加速度 + 脉冲(4字节) + 00 + 00 + 6B
 * @param addr 电机地址
 * @param dir 0=CW(顺时针), 1=CCW(逆时针)
 * @param speed 运行速度 (RPM)
 * @param acc 加速度档位
 * @param pulses 脉冲数（16细分下 3200 = 一圈）
 */
void zdt_motor_move_rel(uint8_t addr, uint8_t dir, uint16_t speed, uint8_t acc, uint32_t pulses);

/**
 * @brief 立即停止（紧急刹车）
 *        指令：addr + FE + 98 + 00 + 6B
 * @param addr 电机地址
 */
void zdt_motor_stop(uint8_t addr);

#endif /* ZDT_MOTOR_H */
