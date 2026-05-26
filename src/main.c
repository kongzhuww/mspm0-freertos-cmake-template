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

/* 定义任务栈大小和优先级 */
#define LED_TASK_STACK_SIZE      256
#define LED_TASK_PRIORITY        1

/* 静态任务控制块和栈空间 */
static StaticTask_t led_task_tcb;
static StackType_t led_task_stack[LED_TASK_STACK_SIZE];

/* 任务函数原型 */
static void vLedBlinkTask(void *pvParameters);


int main(void)
{
    SYSCFG_DL_init();
    debug_uart_init();
    logger_init(); // 初始化异步日志引擎
    
    /* 创建静态任务 - LED闪烁任务 */
    xTaskCreateStatic(
        vLedBlinkTask,              /* 任务函数 */
        "LedBlink",                 /* 任务名称 */
        LED_TASK_STACK_SIZE,        /* 栈深度 */
        NULL,                       /* 参数 */
        LED_TASK_PRIORITY,          /* 优先级 */
        led_task_stack,             /* 栈内存 */
        &led_task_tcb               /* 任务控制块 */
    );
    
    /* 启动调度器 */
    vTaskStartScheduler();
}



/* LED闪烁/按键控制任务实现 */
static void vLedBlinkTask(void *pvParameters)
{
    /* 防止编译器警告 */
    (void) pvParameters;
    // 初始化解耦控制器
    app_context_t app_ctx;
    app_control_init(&app_ctx);
    
    /* 无限循环：极简的 20ms 轮询采样天然防抖 */
    for (;;)
    {
        // 1. 读取底层硬件状态
        uint32_t pin_val = DL_GPIO_readPins(KEY_PORT, KEY_B21_PIN);
        app_key_state_e current_key_state = (pin_val & KEY_B21_PIN) ? APP_KEY_RELEASED : APP_KEY_PRESSED;
        
        // 2. 送入纯逻辑状态机（内部自动处理边缘检测）
        bool toggled = app_control_update(&app_ctx, current_key_state);
        
        // 3. 处理逻辑层输出
        if (toggled || app_ctx.need_toggle)
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
        
        // 4. 释放 CPU 并设定 20ms 采样周期
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}









// 不要修改下面函数


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