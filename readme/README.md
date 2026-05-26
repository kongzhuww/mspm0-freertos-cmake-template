# MSPM0G3507 FreeRTOS CMake 模板仓库

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Build](https://img.shields.io/badge/Build-CMake%20%7C%20Ninja-success)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-orange)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

> **🤖 AI 助手接手指引**：如果您是即将接手本项目的 AI 助手，请**立即查阅**位于 [contexts/context.md](contexts/context.md) 的专属核心上下文文档，以掌握项目深层架构、中断链接规则和异步日志设计。

这是一个专为德州仪器 (TI) MSPM0G3507 微控制器打造的**现代、高效、高工程标准**的 C 语言开发模板。本项目不仅完全脱离了传统 IDE（如 Keil）的束缚，通过 CMake 实现了跨平台构建，更重要的是引入了高级软件工程的实践（硬件与业务逻辑解耦、单元测试、Linter）。

## ✨ 核心特性

- **🚀 现代构建系统**：采用 `CMake` + `Ninja`，彻底抹平环境差异，极速交叉编译输出 `.elf`, `.hex`, `.bin`。
- **🧩 彻底扁平化结构**：源码按 `src`, `include` 分类，所有 SDK 与 FreeRTOS 源码剥离进 `third_party` 依赖区，项目结构异常清爽。
- **🔌 DAPLink 一键烧录**：内建 VS Code 任务，基于原生 OpenOCD，按 `Ctrl+Shift+B` 编译，一键启动 `DAPLink: Flash` 无缝烧录与复位。
- **🧪 业务与硬件绝对解耦**：控制逻辑（状态机）使用纯 C 实现，物理层只作为“提线木偶”接收绝对控制信号，永远不会产生状态翻转不同步的隐患。
- **🧠 极限榨取的资源分配**：彻底禁用 FreeRTOS 动态堆（0 Heap），使用纯静态分配；同时利用 GCC 精确垃圾回收机制，拔除无效组件，将 MCU 的 Flash 与 RAM (BSS) 占用压榨到了极致！任务栈极其精简，绝无内存浪费。
- **💤 微安级浅睡眠待机**：开启 FreeRTOS 空闲钩子，结合 ARM Cortex-M0+ 底层 `__WFI()` 指令，实现“没事就睡觉，有事秒响应”的极低待机功耗。
- **🛡️ 坚不可摧的异常拦截**：底层重定向函数内置 `SCB->ICSR` 寄存器嗅探，杜绝 ISR 中断误调打印引发死锁；栈溢出钩子内置纯硬件轮询打印“遗言”，确保 Hard Fault 死得明明白白。
- **🐞 无缝跨模式调试体验**：彻底分离 Debug/Release 构建。开发期默认使用 `-g -O0` 保证完美断点不乱跳，发布期一键切换 `-O2` 享受极限运行速度。
- **📝 极速异步彩色日志**：内置基于 FreeRTOS StreamBuffer 深度改造的 Logger 框架，实现 `printf` 与 `LOG_INFO` 每次仅消耗 1 次任务唤醒，达到微秒级无阻塞发送。
- **✅ PC 本地单元测试**：内置 `Unity` 测试框架。直接在 Windows/Linux 上本地编译纯逻辑部分进行仿真与断言测试，0 硬件依赖，随时保护重构。
- **🎯 现代 CMake 架构 (Target-Based)**：彻底摒弃传统全局配置（大锅饭）造成的变量污染。将 FreeRTOS 及底层 SDK 剥离为独立静态库，利用 `PUBLIC` 属性优雅地链式传递头文件依赖。
- **🧹 纯净的代码规范护航**：提供 `.clang-format` 统一格式，且 `clang-tidy` 被精准配置为**仅扫描业务源码**，彻底屏蔽第三方库的警告干扰。

## 📂 目录结构

```text
MSPM0_Template/
├── src/                # 业务源码及单片机外设桥接文件
├── include/            # 头文件
├── third_party/        # 第三方依赖库 (TI DriverLib SDK, FreeRTOS, 启动与链接脚本)
├── test/               # PC 端本地单元测试代码 (包含 Unity 框架)
├── contexts/           # 🤖 面向 AI 的核心架构上下文与防坑指南
├── build/              # MCU 交叉编译输出目录 (自动生成)
├── build_test/         # PC 端测试用例编译输出目录 (自动生成)
├── .vscode/            # VS Code 一键编译与烧录任务 (tasks.json)
├── .clang-format       # 统一代码格式化规范
├── CMakeLists.txt      # 现代 CMake 主构建脚本
├── gcc-arm-none-eabi.cmake # 交叉编译工具链配置文件
├── Makefile            # 跨平台 (Linux/macOS) 一键自动化脚本
└── run.bat             # 跨平台 (Windows) 一键自动化脚本
```

## 🛠️ 环境依赖

在开始开发前，请确保您的 PC（Windows/Linux）已安装并配置好以下工具的系统环境变量（Path）：
1. [ARM GCC Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (如 `arm-none-eabi-gcc`)
2. [CMake](https://cmake.org/download/) (>= 3.20)
3. [Ninja](https://ninja-build.org/) 构建工具
4. [OpenOCD](https://openocd.org/) (用于 DAPLink 烧录支持)

## 🏃 快速入门 (MCU 开发)

### 1. 编译固件
在项目根目录下，直接执行我们专门封装的极简构建脚本即可完成交叉编译：

**Windows 环境:**
```bash
.\run.bat build
```

**Linux/macOS 环境:**
```bash
make build
```
*(底层实际执行了 `cmake -DCMAKE_TOOLCHAIN_FILE=...` 和 `cmake -G Ninja --build`)*

编译成功后，在 `build/` 目录下将生成目标固件。

### 2. DAPLink 一键烧录
连接 DAPLink 开发板至 PC。您可以：
- 在 VS Code 中按下快捷键呼出任务列表，选择 `DAPLink: Flash`。
- 或者在命令行直接运行一键烧录脚本：
  ```bash
  .\run.bat flash
  # 或在 Linux/macOS 下使用: make flash
  ```
  *(脚本会自动调用 OpenOCD 并执行 verify, reset 操作)*

### 3. 一键清理
如果需要清除所有编译缓存，可执行：
```bash
.\run.bat clean
# 或在 Linux/macOS 下使用: make clean
```

## 🧪 PC 端单元测试 (无需硬件)

本模板实现了完美的逻辑解耦。您的核心逻辑代码 `app_control.c` 可以直接在宿主机（如您的 Windows 电脑）上通过普通的 GCC 编译并运行测试用例。

执行极简测试指令：

**Windows 环境:**
```bash
.\run.bat test
```

**Linux/macOS 环境:**
```bash
make test
```
该指令会自动完成测试固件的 CMake 配置、编译并立即执行 `run_tests` 二进制文件。

您将看到类似如下的美妙输出：
```text
--- Test Suite Started ---
test_app_control_init_status:PASS
test_app_control_single_press:PASS
test_app_control_long_press:PASS
test_app_control_key_release:PASS
test_app_control_multiple_presses:PASS
-----------------------
5 Tests 0 Failures 0 Ignored
OK
```

## 📄 开源协议
MIT License
