#include "unity.h"
#include "app_control.h"
#include <stddef.h>

static app_context_t ctx;

void test_app_control_init_status(void)
{
    app_control_init(&ctx);
    TEST_ASSERT_EQUAL(APP_LED_OFF, ctx.led_state);
    TEST_ASSERT_EQUAL(APP_KEY_RELEASED, ctx.last_key_state);
    TEST_ASSERT_FALSE(ctx.need_toggle);
}

void test_app_control_single_press(void)
{
    app_control_init(&ctx);
    bool toggled = app_control_update(&ctx, APP_KEY_PRESSED);
    TEST_ASSERT_TRUE(toggled);
    TEST_ASSERT_EQUAL(APP_LED_ON, ctx.led_state);
    TEST_ASSERT_TRUE(ctx.need_toggle);
}

void test_app_control_long_press(void)
{
    app_control_init(&ctx);
    app_control_update(&ctx, APP_KEY_PRESSED);
    bool toggled = app_control_update(&ctx, APP_KEY_PRESSED);
    TEST_ASSERT_FALSE(toggled);
    TEST_ASSERT_EQUAL(APP_LED_ON, ctx.led_state);
    TEST_ASSERT_FALSE(ctx.need_toggle);
}

void test_app_control_key_release(void)
{
    app_control_init(&ctx);
    app_control_update(&ctx, APP_KEY_PRESSED);
    bool toggled = app_control_update(&ctx, APP_KEY_RELEASED);
    TEST_ASSERT_FALSE(toggled);
    TEST_ASSERT_EQUAL(APP_LED_ON, ctx.led_state);
    TEST_ASSERT_FALSE(ctx.need_toggle);
}

void test_app_control_multiple_presses(void)
{
    app_control_init(&ctx);
    app_control_update(&ctx, APP_KEY_PRESSED);
    TEST_ASSERT_EQUAL(APP_LED_ON, ctx.led_state);

    app_control_update(&ctx, APP_KEY_RELEASED);

    app_control_update(&ctx, APP_KEY_PRESSED);
    TEST_ASSERT_EQUAL(APP_LED_OFF, ctx.led_state);
    TEST_ASSERT_TRUE(ctx.need_toggle);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_app_control_init_status);
    RUN_TEST(test_app_control_single_press);
    RUN_TEST(test_app_control_long_press);
    RUN_TEST(test_app_control_key_release);
    RUN_TEST(test_app_control_multiple_presses);

    return UNITY_END();
}
