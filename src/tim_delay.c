#include "tim_delay.h"
#include "FreeRTOS.h"
#include "task.h"

// 最大延时为65535微秒，即65.535毫秒
void delay_us(uint16_t us)
{
    // 【防御性护航】RTOS 环境下，严禁超过 2ms 的死循环延时，防止霸占 CPU 饿死其他任务！
    // 超过 2000us 请规范使用 vTaskDelay！
    configASSERT(us <= 2000);

    DL_TimerA_setLoadValue(TIM_delay_us_INST, us);   // 设置自动重载值（计数目标）
    DL_TimerA_setTimerCount(TIM_delay_us_INST, us);      // 计数器初始化为t（向下计数从t开始）
    DL_TimerA_startCounter(TIM_delay_us_INST);            // 启动定时器

    // 等待计数器减到0（完成t次计数，耗时t us）
    while (DL_TimerA_getTimerCount(TIM_delay_us_INST) != 0)
    {
        // 空循环，持续检查计数器值
    }
}