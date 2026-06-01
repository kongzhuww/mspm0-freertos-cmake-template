#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

// 抽象 LED 状态
typedef enum {
    APP_LED_OFF = 0,
    APP_LED_ON = 1
} app_led_state_e;

// 抽象按键状态
typedef enum {
    APP_KEY_RELEASED = 1,  // 高电平
    APP_KEY_PRESSED = 0    // 低电平
} app_key_state_e;

// 控制逻辑的上下文状态结构体
typedef struct {
    app_led_state_e led_state;  // LED 当前状态
    app_key_state_e last_key_state; // 上一次读取的按键状态
} app_context_t;

/**
 * @brief 初始化应用程序控制器上下文
 * @param ctx 待初始化的控制器上下文
 */
void app_control_init(app_context_t *ctx, app_key_state_e init_key_state);

/**
 * @brief 更新按键控制逻辑状态机（纯算法，无硬件依赖）
 * @param ctx 控制器上下文
 * @param key_state 当前按键的电平状态
 * @return 发生翻转时返回 true
 */
bool app_control_update(app_context_t *ctx, app_key_state_e key_state);

#endif /* APP_CONTROL_H */
