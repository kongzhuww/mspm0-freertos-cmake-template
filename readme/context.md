# 项目核心上下文 (Project Core Context)

> **最高指令提示 (Supreme Directive)**: 任何 AI 助手在接手本工程或开始新会话时，必须优先阅读本文件以掌握系统级架构和构建防坑指南。

## 1. 工程基础架构
- **硬件平台**: 德州仪器 (TI) MSPM0G3507 (ARM Cortex-M0+ 内核)
- **构建系统**: 纯 `CMake` + `Ninja`，彻底脱离传统 IDE（无 Keil / CCS 工程文件）。
- **交叉编译器**: ARM GCC Toolchain (`arm-none-eabi-gcc`)。
- **烧录调试**: `OpenOCD` + CMSIS-DAP 调试器。

## 2. 软件设计原则与目录规范
工程采用严格的模块化与解耦设计：
- `src/` & `include/`: 纯业务逻辑与应用程序代码。受 `clang-tidy` 静态代码扫描保护。
- `third_party/`: 包括所有底层的 TI DriverLib SDK 源码、启动/链接脚本、以及 FreeRTOS 源码。**此目录被打包为独立的静态库 (`third_party_lib.a`)，以完全屏蔽 `clang-tidy` 扫描造成的性能开销和杂乱警告。**
- `test/`: 基于 `Unity` 的 PC 端本地单元测试（直接在宿主机编译运行纯逻辑代码）。

## 3. 关键架构设计与“大坑”防范（重要！）

### 3.1 FreeRTOS 静态库与链接器精确裁剪 (极限 Flash 优化)
- **原理**: 由于 FreeRTOS 的心跳 (`SysTick_Handler`) 和上下文切换 (`SVC_Handler`, `PendSV_Handler`) 是**硬件中断函数**，应用层不会显示调用。常规链接可能会导致 GCC 的 `--gc-sections` 优化剔除这些中断向量。
- **曾用解法**: 以前为了防止被剔除，在链接时使用了暴力的 `-Wl,--whole-archive` 参数，但这导致整个库里所有未使用的 FreeRTOS 组件（如队列、定时器等）全部被塞进 Flash 中，造成严重的空间浪费。
- **现行最优解法**: 现已将 `startup_mspm0g350x_gcc.c` 从库代码中剥离，直接加入 `APP_SOURCES` 作为主程序的一部分！由于中断向量表位于主程序中，链接器会顺藤摸瓜，精确且**仅抓取**真正需要的 FreeRTOS 库文件。去除了 `--whole-archive` 捆绑，Flash 尺寸得以大幅下降！

### 3.2 异步非阻塞日志框架 (Async Logger)
- **现状**: 系统自带专业彩色异步日志系统（见 `src/logger.c` 及 `include/logger.h`）。
- **机制**:
  - `FreeRTOSConfig.h` 开启了 `configUSE_STREAM_BUFFERS 1`。
  - 重构了 GCC 的底层重定向 `_write`。当调用 `printf` 或 `LOG_INFO` 宏时，数据受 Mutex 保护瞬间推入 512 字节的环形缓冲区 (StreamBuffer)，实现了微秒级的零阻塞。
  - 系统启动了一个静态优先级为 0 (空闲级) 的 `LoggerTask` 后台任务，在不抢占任何核心业务的情况下，通过底层 UART 轮询完成物理发送。
- **中断防死锁保护 (重点!)**: RTOS 中的 `StreamBuffer` 阻塞调用严禁在中断上下文 (ISR) 中使用。我们在 `_write` 函数底层通过嗅探 ARM Cortex-M0+ 的 `SCB->ICSR` 寄存器自动拦截了中断中的打印请求。尽管如此，依然**严禁在硬件中断中调用 `LOG_INFO`**！

### 3.3 极致防错与低功耗架构
- **栈溢出“遗言”机制**: 若开发中遇到单片机突然卡死，通常是由于任务栈溢出 (`Stack Overflow`) 导致。系统已重写了 `vApplicationStackOverflowHook`，在死机前会通过极其可靠的**纯硬件串口轮询**将越界任务的名称打印出来。
- **微安级休眠**: 系统开启了 `configUSE_IDLE_HOOK`，任何时候当 RTOS 没有高优先级任务时（例如等待 20ms 的 Delay），将自动执行 `__WFI()` 指令挂起 CPU 时钟。这种设计将待机功耗降至了极限。

### 3.4 GCC Newlib-Nano 库与浮点/换行处理
- 编译启用了极简裸机标准库 `--specs=nano.specs --specs=nosys.specs` 以节省 Flash。
- 在 `_write` 重定向底层（`UART0_Debug.c`），已经实现了 `\n` 自动转换为 `\r\n` 的逻辑，所以在 `printf` 和日志宏中只需使用 `\n` 即可。

## 4. 开发工作流指南
- **新外设接入**: 硬件层面上由于去除了官方的 `syscfg` GUI 自动生成工具依赖，所有的引脚与外设初始化目前在 `src/ti_msp_dl_config.c` 中直接管理。
- **编写逻辑代码**: 请务必参考 `app_control.c`，把状态机剥离到纯 C 层，然后在 `test_app_control.c` 中编写单元测试。
- **代码检查**: 主工程与测试工程均在 CMake 中挂载了 `clang-tidy`（配置于 `MY_CLANG_TIDY_CMD`）。新代码如有不合规将会报黄。
