#ifndef DUMMY_SENSOR_H
#define DUMMY_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 假装初始化某个传感器
 * @return 成功返回 true
 */
bool dummy_sensor_init(void);

/**
 * @brief 假装读取数据
 * @return 随机数据
 */
int32_t dummy_sensor_read(void);

#endif /* DUMMY_SENSOR_H */
