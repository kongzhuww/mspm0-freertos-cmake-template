/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* TI includes */
#include "ti_msp_dl_config.h"

#include "board_led.h"
#include "UART0_Debug.h"
#include "logger.h"
#include "tim_delay.h"
#include "app_control.h"
#include "oled.h"

/* 定义任务栈大小和优先级 */
#define LED_TASK_STACK_SIZE      512
#define LED_TASK_PRIORITY        1

#define OLED_TASK_STACK_SIZE     512
#define OLED_TASK_PRIORITY       2

/* 静态任务控制块和栈空间 */
static StaticTask_t led_task_tcb;
static StackType_t led_task_stack[LED_TASK_STACK_SIZE];
static TaskHandle_t led_task_handle = NULL;

static StaticTask_t oled_task_tcb;
static StackType_t oled_task_stack[OLED_TASK_STACK_SIZE];

/* 任务函数原型 */
static void vLedBlinkTask(void *pvParameters);
static void vOledDisplayTask(void *pvParameters);


int main(void)
{
    SYSCFG_DL_init();
    debug_uart_init();
    logger_init(); // 初始化异步日志引擎
    
    /* 创建静态任务 - LED闪烁任务 */
    led_task_handle = xTaskCreateStatic(
        vLedBlinkTask,              /* 任务函数 */
        "LedBlink",                 /* 任务名称 */
        LED_TASK_STACK_SIZE,        /* 栈深度 */
        NULL,                       /* 参数 */
        LED_TASK_PRIORITY,          /* 优先级 */
        led_task_stack,             /* 栈内存 */
        &led_task_tcb               /* 任务控制块 */
    );
    
    /* 创建静态任务 - OLED 显示任务 */
    xTaskCreateStatic(
        vOledDisplayTask,           /* 任务函数 */
        "OledDisplay",              /* 任务名称 */
        OLED_TASK_STACK_SIZE,       /* 栈深度 */
        NULL,                       /* 参数 */
        OLED_TASK_PRIORITY,         /* 优先级 */
        oled_task_stack,            /* 栈内存 */
        &oled_task_tcb              /* 任务控制块 */
    );
    
    /* 启动调度器 */
    vTaskStartScheduler();
}



/* OLED 显示任务实现 */
static void vOledDisplayTask(void *pvParameters)
{
    (void) pvParameters;
    
    // 初始化 OLED 屏幕（包含 SSD1306 配置序列 + 清屏）
    oled_init();
    
    // 在第一行显示 "Hello, World"
    oled_show_string(0, 0, "Hello, World", 16);
    
    LOG_INFO("OLED initialized, displaying 'Hello, World'");
    
    // 显示完成后任务挂起，不再占用 CPU
    vTaskSuspend(NULL);
}

/* LED闪烁/按键控制任务实现 */
static void vLedBlinkTask(void *pvParameters)
{
    /* 防止编译器警告 */
    (void) pvParameters;
    // 初始化解耦控制器（基于物理引脚上电时的真实初始状态进行初始化，避免上电误触发）
    uint32_t init_pin = DL_GPIO_readPins(KEY_PORT, KEY_B21_PIN);
    app_key_state_e init_key_state = (init_pin & KEY_B21_PIN) ? APP_KEY_RELEASED : APP_KEY_PRESSED;
    app_context_t app_ctx;
    app_control_init(&app_ctx, init_key_state);
    
    // [修复 Bug #1] 调度器已正常启动，此时安全地清除挂起标志并使能按键 NVIC 中断
    NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
    
    for (;;)
    {
        // 等唤醒通知
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // 1. 物理消抖：等待 10ms 避开机械抖动区，在此期间任务挂起并让出 CPU
        // 注：此时引脚中断在 ISR 中已被立刻禁用，因此延时期间绝对不会累加多余通知
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // 2. 读取消抖稳定后的最终硬件状态
        uint32_t pin_val = DL_GPIO_readPins(KEY_PORT, KEY_B21_PIN);
        app_key_state_e current_key_state = (pin_val & KEY_B21_PIN) ? APP_KEY_RELEASED : APP_KEY_PRESSED;
        
        // 3. 清除在消抖延时期间由于接触不良或物理回弹挂起的硬件中断标志
        DL_GPIO_clearInterruptStatus(KEY_PORT, KEY_B21_PIN);
        
        // 4. 保底清空在此期间可能积累的多余任务通知，确保下一次等待为干净状态
        ulTaskNotifyTake(pdTRUE, 0);
        
        // 5. 重新使能按键引脚中断，准备下一次检测
        DL_GPIO_enableInterrupt(KEY_PORT, KEY_B21_PIN);
        
        // 6. 送入纯逻辑状态机（内部自动处理边缘检测）
        bool toggled = app_control_update(&app_ctx, current_key_state);
        
        // 7. 处理逻辑层输出
        if (toggled)
        {
            /* 物理层绝对服从逻辑层状态（不使用相对翻转） */
            if (app_ctx.led_state == APP_LED_ON) {
                board_led_on();
            } else {
                board_led_off();
            }
            
            // 打印按键触发日志
            LOG_INFO("KEY (PB21) State Machine Triggered! LED toggled to %s", (app_ctx.led_state == APP_LED_ON) ? "ON" : "OFF");
        }
    }
}

/*
 * GPIOB (KEY_B21 所在组) 中断服务函数
 * 【中断分组映射说明】MSPM0 的 GPIOB 中断被路由到 INT_GROUP1，因此 ISR 函数名必须是
 * GROUP1_IRQHandler（由启动文件中的向量表决定）。对应的 NVIC IRQn 为 GPIOB_INT_IRQn。
 * 如果更换了 GPIO 端口，务必同步更新此 ISR 函数名与 NVIC_EnableIRQ 的 IRQn 参数。
 */
void GROUP1_IRQHandler(void)
{
    // 获取 GPIOB 的中断状态
    uint32_t pending = DL_GPIO_getEnabledInterruptStatus(GPIOB, KEY_B21_PIN);
    
    if (pending & KEY_B21_PIN)
    {
        // 1. [修复 Bug #2] 立即在中断层禁用引脚中断，锁死硬件状态，防范调度延迟期间的中断累加和吞键
        DL_GPIO_disableInterrupt(GPIOB, KEY_B21_PIN);
        
        // 2. 清除中断标志
        DL_GPIO_clearInterruptStatus(GPIOB, KEY_B21_PIN);
        
        // 3. 通知 LED 任务去处理电平变化
        if (led_task_handle != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(led_task_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/*===========================================================================
 * FreeRTOS 系统回调（钩子函数 / 静态内存供给）
 * 这些函数由 FreeRTOS 内核自动调用，签名不可更改。
 *===========================================================================*/

// 当 configUSE_IDLE_HOOK 设为 1 时，系统空闲会调用此钩子
void vApplicationIdleHook(void)
{
    // 注意：开启 Tickless Idle (configUSE_TICKLESS_IDLE=1) 后，FreeRTOS 的
    // vPortSuppressTicksAndSleep 会独立接管深睡逻辑，此处的 __WFI() 仅作为保底
    __WFI();
}

// 预休眠处理钩子：在系统即将进入 Tickless Idle 深睡前自动被 FreeRTOS 调用
void vPreSleepProcessing(unsigned long *ulExpectedIdleTime)
{
    // 硬件级雷达扫描：如果检测到连接了调试器(DAPLink)，强行放弃深度休眠
    // Cortex-M 系列通用 DHCSR 寄存器地址为 0xE000EDF0，最低位 (bit 0) 为 C_DEBUGEN
    // 【可移植性注意】ARMv6-M 规范中 CoreDebug 寄存器在 non-debug 状态下的访问行为
    // 是 IMPLEMENTATION DEFINED。TI MSPM0 系列支持安全读取，但移植到其他 Cortex-M0+
    // 芯片时需确认厂商是否允许非调试态访问 DHCSR，否则可能触发 Bus Fault。
    volatile uint32_t *dhcsr = (volatile uint32_t *)0xE000EDF0;
    if ((*dhcsr & 1) != 0) {
        *ulExpectedIdleTime = 0; // 预期休眠时间清零，FreeRTOS 将中止休眠流程，保证烧录/调试畅通无阻
    }
}

/*-----------------------------------------------------------*/

// 如果启用了静态内存分配功能
#if (configSUPPORT_STATIC_ALLOCATION == 1)
/*
 *  ======== vApplicationGetIdleTaskMemory ========
 *  当启用静态内存分配时，应用程序必须提供此回调函数
 *  供空闲任务使用。
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    // 静态分配空闲任务的任务控制块
    static StaticTask_t xIdleTaskTCB;
    // 静态分配空闲任务的栈空间，大小由配置决定
    static StackType_t uxIdleTaskStack[configIDLE_TASK_STACK_DEPTH];

    // 返回任务控制块的指针
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    // 返回任务栈的指针
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    // 返回任务栈的大小
    *pulIdleTaskStackSize   = configIDLE_TASK_STACK_DEPTH;
}

// 如果启用了定时器功能
#if (configUSE_TIMERS == 1)
/*
 *  ======== vApplicationGetTimerTaskMemory ========
 *  当启用静态内存分配并使用定时器时，应用程序必须提供
 *  此回调函数供定时器服务任务使用。
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    // 静态分配定时器任务的任务控制块
    static StaticTask_t xTimerTaskTCB;
    // 静态分配定时器任务的栈空间，大小由配置决定
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    // 返回任务控制块的指针
    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    // 返回任务栈的指针
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    // 返回任务栈的大小
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}
#endif

#endif

// 如果启用了栈溢出检测功能
#if (configCHECK_FOR_STACK_OVERFLOW)
/*
 *  ======== vApplicationStackOverflowHook ========
 *  当启用栈溢出检测时，应用程序必须提供一个栈溢出钩子函数。
 *  此默认钩子函数被声明为弱符号，除非应用程序专门提供自己的
 *  钩子函数，否则将默认使用此函数。
 */
// IAR编译器的弱符号声明方式
#if defined(__IAR_SYSTEMS_ICC__)
__weak void vApplicationStackOverflowHook(
    TaskHandle_t pxTask, char *pcTaskName)
// TI编译器的弱符号声明方式
#elif (defined(__TI_COMPILER_VERSION__))
#pragma WEAK(vApplicationStackOverflowHook)
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
// GCC编译器的弱符号声明方式
#elif (defined(__GNUC__) || defined(__ti_version__))
void __attribute__((weak))
vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
#endif
{
    // [修复] 立即彻底屏蔽所有硬件中断，保证受损系统在纯净的单核状态下把遗言打印出来
    __disable_irq();

    // [致命错误护航] 发生栈溢出时，RTOS 环境可能已完全崩塌，绝对不能调用任何 RTOS API。
    // 使用最底层的纯硬件死等轮询方式，把导致崩溃的任务名称“遗言”发出去。
    debug_uart_send_string("\r\n[FATAL ERROR] FreeRTOS Stack Overflow detected in task: ");
    debug_uart_send_string(pcTaskName);
    debug_uart_send_string("\r\nSYSTEM HALTED.\r\n");

    // 默认实现：检测到栈溢出时进入无限循环，卡死在此处以便连接 DAPLink 观察 Call Stack
    while (1) {
    }
}
#endif