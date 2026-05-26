#include "board_led.h"

void board_led_on(void)
{
    DL_GPIO_setPins(LED_PORT, LED_B14_PIN);  // 输出高电平
}

void board_led_off(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_B14_PIN);  // 输出低电平
}
