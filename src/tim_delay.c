#include "tim_delay.h"
#include "FreeRTOS.h"
#include "task.h"

// 最大延时为65535微秒，即65.535毫秒
void delay_us(uint16_t us)
{
    // 【防御性护航】RTOS 环境下，严禁超过 2ms 的死循环延时，防止霸占 CPU 饿死其他任务！
    // 超过 2000us 请规范使用 vTaskDelay！
    configASSERT(us <= 2000);

    // 0 微秒无需延时，直接返回（避免 One-Shot 模式下 Load=0 的未定义行为）
    if (us == 0) {
        return;
    }

    taskENTER_CRITICAL(); // [修复] 仅在硬件参数配置阶段关闭中断，锁定资源独占访问

    // 确保定时器处于停止状态，防止连续快速调用时前一次计数尚未结束
    DL_TimerA_stopCounter(TIM_delay_us_INST);

    // 清除 Zero Event 中断挂起标志位，防范残留脏状态
    DL_TimerA_clearInterruptStatus(TIM_delay_us_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT);

    DL_TimerA_setLoadValue(TIM_delay_us_INST, us);   // 设置自动重载值（计数目标）
    DL_TimerA_setTimerCount(TIM_delay_us_INST, us);  // 计数器初始化为t（向下计数从t开始）
    DL_TimerA_startCounter(TIM_delay_us_INST);        // 启动定时器

    taskEXIT_CRITICAL(); // 配置完毕立刻退出临界区，开中断以保障系统硬实时响应！

    // 在开中断状态下阻塞死等（完成t次计数，耗时t us），保障系统滴答中断与高频串口中断零延迟响应
    while (DL_TimerA_getRawInterruptStatus(TIM_delay_us_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT) == 0)
    {
        // 空循环
    }

    taskENTER_CRITICAL(); // 仅在停止硬件资源时关中断
    // 使用完毕，停止定时器，释放硬件资源
    DL_TimerA_stopCounter(TIM_delay_us_INST);
    taskEXIT_CRITICAL();
}