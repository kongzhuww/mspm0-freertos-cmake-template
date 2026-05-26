#include "tim_delay.h"

// 最大延时为65535微秒，即65.535毫秒
void delay_us(uint16_t us)
{
    DL_TimerA_setLoadValue(TIM_delay_us_INST, us);   // 设置自动重载值（计数目标）
    DL_TimerA_setTimerCount(TIM_delay_us_INST, us);      // 计数器初始化为t（向下计数从t开始）
    DL_TimerA_startCounter(TIM_delay_us_INST);            // 启动定时器

    // 等待计数器减到0（完成t次计数，耗时t us）
    while (DL_TimerA_getTimerCount(TIM_delay_us_INST) != 0)
    {
        // 空循环，持续检查计数器值
    }
     // 停止定时器（单次模式会自动停止，但显式停止更安全）
     //DL_TimerA_stopCounter(TIM_delay_us_INST);             // 停止定时器
}