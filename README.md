# MSPM0G3507 FreeRTOS CMake 模板仓库

这是一个专为德州仪器 (TI) MSPM0G3507 微控制器打造的**现代、高效、高工程标准**的 C 语言开发模板。本项目不仅完全脱离了传统 IDE（如 Keil）的束缚，通过 CMake 实现了跨平台构建，更重要的是引入了高级软件工程的实践（硬件与业务逻辑解耦、单元测试、Linter）。

## ✨ 核心特性

- **🚀 现代构建系统**：采用 `CMake` + `Ninja`，极速交叉编译输出 `.elf`, `.hex`, `.bin`。
- **🧩 彻底扁平化结构**：源码按 `src`, `include` 分类，所有 SDK 与 FreeRTOS 源码剥离进 `third_party` 依赖区，项目结构异常清爽。
- **🔌 DAPLink 一键烧录**：内建 VS Code 任务，基于原生 OpenOCD，按 `Ctrl+Shift+B` 编译，一键启动 `DAPLink: Flash` 无缝烧录与复位。
- **🧪 业务与硬件解耦**：控制逻辑（按键消抖与 LED 状态机）使用纯 C 实现，完全不依赖硬件库头文件。
- **✅ PC 本地单元测试**：内置 `Unity` 测试框架。直接在 Windows/Linux 上本地编译纯逻辑部分进行仿真与断言测试，无需连接开发板。
- **🧹 代码规范护航**：提供 `.clang-format` 标准格式化，并在 `CMakeLists.txt` 中集成了 `clang-tidy` 静态分析支持。

## 📂 目录结构

```text
MSPM0_Template/
├── src/                # 业务源码及单片机外设桥接文件
├── include/            # 头文件
├── third_party/        # 第三方依赖库 (TI DriverLib SDK, FreeRTOS, 启动与链接脚本)
├── test/               # PC 端本地单元测试代码 (包含 Unity 框架)
├── .vscode/            # VS Code 一键编译与烧录任务 (tasks.json)
├── .clang-format       # 统一代码格式化规范
├── CMakeLists.txt      # 现代 CMake 主构建脚本
└── gcc-arm-none-eabi.cmake # 交叉编译工具链配置文件
```

## 🛠️ 环境依赖

在开始开发前，请确保您的 PC（Windows/Linux）已安装并配置好以下工具的系统环境变量（Path）：
1. [ARM GCC Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (如 `arm-none-eabi-gcc`)
2. [CMake](https://cmake.org/download/) (>= 3.20)
3. [Ninja](https://ninja-build.org/) 构建工具
4. [OpenOCD](https://openocd.org/) (用于 DAPLink 烧录支持)

## 🏃 快速入门 (MCU 开发)

### 1. 编译固件
在项目根目录下，执行以下命令进行交叉编译（或者直接在 VS Code 中运行 `CMake: Configure` 和 `CMake: Build` 任务）：
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=gcc-arm-none-eabi.cmake -G Ninja -B build
cmake --build build
```
编译成功后，在 `build/` 目录下将生成目标固件。

### 2. DAPLink 一键烧录
连接 DAPLink 开发板至 PC。您可以：
- 在 VS Code 中按下快捷键呼出任务列表，选择 `DAPLink: Flash`。
- 或在命令行直接运行：
  ```bash
  openocd -f interface/cmsis-dap.cfg -c "adapter speed 2000" -f target/ti_mspm0.cfg -c "program build/MSPM0G3507_FreeRTOS.elf verify reset exit"
  ```

## 🧪 PC 端单元测试 (无需硬件)

本模板实现了完美的逻辑解耦。您的核心逻辑代码 `app_control.c` 可以直接在宿主机（如您的 Windows 电脑）上通过普通的 GCC 编译并运行测试用例。

执行以下命令：
```bash
# 配置为本地编译模式
cmake -B build-test
# 编译测试用例
cmake --build build-test
# 运行单元测试
./build-test/run_tests
```
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
