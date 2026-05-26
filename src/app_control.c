#include "app_control.h"
#include <stddef.h>

void app_control_init(app_context_t *ctx)
{
    if (ctx != NULL) {
        ctx->led_state = APP_LED_OFF;
        ctx->last_key_state = APP_KEY_RELEASED;
    }
}

bool app_control_update(app_context_t *ctx, app_key_state_e key_state)
{
    if (ctx == NULL) {
        return false;
    }

    bool toggled = false;

    // 状态转换：检测按键按下边缘（下降沿：1 -> 0）
    if (ctx->last_key_state == APP_KEY_RELEASED && key_state == APP_KEY_PRESSED) {
        // 翻转 LED 状态
        if (ctx->led_state == APP_LED_OFF) {
            ctx->led_state = APP_LED_ON;
        } else {
            ctx->led_state = APP_LED_OFF;
        }
        toggled = true;
    }

    ctx->last_key_state = key_state;
    return toggled;
}
