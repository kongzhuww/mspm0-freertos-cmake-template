#include "dummy_sensor.h"

// 如果使用了 FreeRTOS 或者是底层的驱动库，这里可以自由包含，无需担心污染外部
#if defined(__GNUC__) && defined(__MSPM0G3507__)
#include "ti_msp_dl_config.h"
#include "FreeRTOS.h"
#include "task.h"
#endif

bool dummy_sensor_init(void)
{
    // 假装初始化硬件
    return true;
}

int32_t dummy_sensor_read(void)
{
    // 假装返回一个固定数据
    return 42;
}
