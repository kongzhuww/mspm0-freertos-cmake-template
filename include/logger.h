#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

/* ANSI Colors */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* 
 * 专业的彩色日志宏
 * 注意：底层 _write 已实现自动补全 \r，此处只需 \n 即可
 */
#define LOG_INFO(fmt, ...)  printf(ANSI_COLOR_GREEN "[INFO] " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  printf(ANSI_COLOR_YELLOW "[WARN] " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) printf(ANSI_COLOR_RED "[ERROR] " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

/**
 * @brief 初始化异步日志系统
 * 创建 StreamBuffer、互斥锁并启动后台打印任务。
 */
void logger_init(void);

#endif /* LOGGER_H */
