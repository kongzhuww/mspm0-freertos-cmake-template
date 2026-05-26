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

### 3.1 FreeRTOS 静态库与链接器陷阱
- **现状**: FreeRTOS 源码与芯片启动文件 (`startup_mspm0g350x_gcc.c`) 被作为静态库 `third_party_lib` 编译。
- **大坑警告**: 由于 FreeRTOS 的心跳 (`SysTick_Handler`) 和上下文切换 (`SVC_Handler`, `PendSV_Handler`) 是**硬件中断函数**，应用层不会显示调用。如果使用常规静态库链接，GCC 链接器 `--gc-sections` 优化规则会**彻底剔除这些中断向量**，导致启动调度器后系统立刻挂死！
- **解决方案**: 在 `CMakeLists.txt` 中，链接 `third_party_lib` 时**必须**使用强制保留所有符号的包裹参数：
  ```cmake
  target_link_libraries(${PROJECT_NAME}.elf PRIVATE -Wl,--whole-archive third_party_lib -Wl,--no-whole-archive)
  ```

### 3.2 异步非阻塞日志框架 (Async Logger)
- **现状**: 系统自带专业彩色异步日志系统（见 `src/logger.c` 及 `include/logger.h`）。
- **机制**:
  - `FreeRTOSConfig.h` 开启了 `configUSE_STREAM_BUFFERS 1`。
  - 重构了 GCC 的底层重定向 `_write`。当调用 `printf` 或 `LOG_INFO` 宏时，数据受 Mutex 保护瞬间推入 512 字节的环形缓冲区 (StreamBuffer)，实现了微秒级的零阻塞。
  - 系统启动了一个静态优先级为 0 (空闲级) 的 `LoggerTask` 后台任务，在不抢占任何核心业务的情况下，通过底层 UART 轮询完成物理发送。
- **注意事项**: 任何新增任务都可以安全、高频地调用 `LOG_INFO` 等宏，**不会**因为串口的硬件传输耗时而阻塞业务。

### 3.3 GCC Newlib-Nano 库与浮点/换行处理
- 编译启用了极简裸机标准库 `--specs=nano.specs --specs=nosys.specs` 以节省 Flash。
- 在 `_write` 重定向底层（`UART0_Debug.c`），已经实现了 `\n` 自动转换为 `\r\n` 的逻辑，所以在 `printf` 和日志宏中只需使用 `\n` 即可。

## 4. 开发工作流指南
- **新外设接入**: 硬件层面上由于去除了官方的 `syscfg` GUI 自动生成工具依赖，所有的引脚与外设初始化目前在 `src/ti_msp_dl_config.c` 中直接管理。
- **编写逻辑代码**: 请务必参考 `app_control.c`，把状态机剥离到纯 C 层，然后在 `test_app_control.c` 中编写单元测试。
- **代码检查**: 主工程与测试工程均在 CMake 中挂载了 `clang-tidy`（配置于 `MY_CLANG_TIDY_CMD`）。新代码如有不合规将会报黄。
