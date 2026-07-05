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
 * @param reverse 是否反色显示 (1:反色, 0:正常)
 */
void oled_show_char(uint8_t x, uint8_t y, char chr, uint8_t size, uint8_t reverse);

/**
 * @brief 在 OLED 屏幕上显示字符串
 * @param x 列坐标 (0 ~ 127)
 * @param y 行坐标/页坐标 (0 ~ 7)
 * @param str 要显示的字符串
 * @param size 字体大小 (16: 8x16, 12: 6x8)
 * @param reverse 是否反色显示 (1:反色, 0:正常)
 */
void oled_show_string(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t reverse);

/**
 * @brief 显示一帧128x64像素的小猫跳舞画面并支持水平平移
 * @param frame_data 指向当前帧图像数据（8页x128列）
 * @param x_offset 水平偏移量（用于实现滚动动画）
 */
void oled_draw_cat_frame(const uint8_t frame_data[8][128], int16_t x_offset);

#endif /* OLED_H */
