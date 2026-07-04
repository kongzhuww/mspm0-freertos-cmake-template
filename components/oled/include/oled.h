#ifndef OLED_H
#define OLED_H

#include <stdint.h>

/**
 * @brief 初始化 OLED 显示屏
 */
void oled_init(void);

/**
 * @brief 清除整个 OLED 屏幕
 */
void oled_clear(void);

/**
 * @brief 在 OLED 屏幕上显示一个字符
 * @param x 列坐标 (0 ~ 127)
 * @param y 行坐标/页坐标 (0 ~ 7)
 * @param chr 要显示的 ASCII 字符
 * @param size 字体大小 (16: 8x16, 12: 6x8)
 */
void oled_show_char(uint8_t x, uint8_t y, char chr, uint8_t size);

/**
 * @brief 在 OLED 屏幕上显示字符串
 * @param x 列坐标 (0 ~ 127)
 * @param y 行坐标/页坐标 (0 ~ 7)
 * @param str 要显示的字符串
 * @param size 字体大小 (16: 8x16, 12: 6x8)
 */
void oled_show_string(uint8_t x, uint8_t y, const char *str, uint8_t size);

#endif /* OLED_H */
