cmake_minimum_required(VERSION 3.20)
project(MSPM0G3507_FreeRTOS C ASM CXX)

# 支持 C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 使用默认编译错误输出，避免 clangd 解析 GCC 专用 JSON 诊断参数时报错

# 启用 clang-tidy (如果可用)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    # 不再全局设置，而是保存配置，后续按 Target 应用
    set(MY_CLANG_TIDY_CMD "${CLANG_TIDY_EXE};-checks=-*,readability-*,bugprone-*")
endif()

# ==============================================================================
# 编译器标志设置（必须在 add_subdirectory 之前，确保组件库继承正确的 CPU 架构）
# ==============================================================================
if(CMAKE_CROSSCOMPILING)
    set(CPU_FLAGS "-mcpu=cortex-m0plus -mthumb")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS} -g -ffunction-sections -fdata-sections -Wall")
    set(CMAKE_C_FLAGS_DEBUG "-O0")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -flto")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_FLAGS} -x assembler-with-cpp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections -flto")
endif()

# ==============================================================================
# 引入比赛专用驱动组件库 (Components)
# ==============================================================================
# 这行代码会将 components/ 目录下的所有独立外设驱动加入到构建树中
add_subdirectory(components)

if(CMAKE_CROSSCOMPILING)
    # --------- 交叉编译到单片机 ---------

    # (编译器标志已在 add_subdirectory 之前设置，此处不再重复)


    # 业务源码 (启动文件一并算入，保证精确链接)
    set(APP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/devices/startup_mspm0g350x_gcc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ti_msp_dl_config.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/board_led.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0_Debug.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tim_delay.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app_control.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.c
    )

    # 第三方库源码 (FreeRTOS及其他纯净库代码)
    set(THIRD_PARTY_SOURCES
        # FreeRTOS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/croutine.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/event_groups.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/stream_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/tasks.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/timers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/portable/GCC/ARM_CM0/port.c
    )

    # 链接脚本
    set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/devices/mspm0g3507.lds")

    # 定义 TI DriverLib 驱动源码
    set(DRIVERLIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_adc12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_aes.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_aesadv.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_comp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_crc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_crcp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_dac12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_dma.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_flashctl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_gpamp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_i2c.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_iwdt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_keystorectl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_lcd.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_lfss.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_mathacl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_mcan.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_opa.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_rtc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_rtc_a.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_rtc_b.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_rtc_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_scratchpad.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_spi.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_tamperio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_timer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_timera.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_timerg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_trng.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_uart.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_vref.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/dl_wwdt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/m0p/dl_factoryregion.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/m0p/dl_interrupt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0g1x0x_g3x0x.c
    )

    # 将 driverlib 编译为静态库，避免预编译库丢失的问题
    add_library(driverlib STATIC ${DRIVERLIB_SOURCES})
    target_compile_definitions(driverlib PUBLIC -D__MSPM0G3507__)
    target_include_directories(driverlib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/third_party/CMSIS/Core/Include
    )

    # 将第三方代码编译为静态库
    add_library(third_party_lib STATIC ${THIRD_PARTY_SOURCES})
    # 为第三方库配置专属的宏和头文件，并设为 PUBLIC 传递给依赖它的主程序
    target_compile_definitions(third_party_lib PUBLIC -D__MSPM0G3507__)
    target_include_directories(third_party_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/third_party/CMSIS/Core/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/portable/GCC/ARM_CM0
    )

    # 编译主程序
    add_executable(${PROJECT_NAME}.elf ${APP_SOURCES})

    # 找到并配置 clang-tidy 静态检查
    if(CLANG_TIDY_EXE)
        # set_target_properties(${PROJECT_NAME}.elf PROPERTIES C_CLANG_TIDY "${MY_CLANG_TIDY_CMD}")
    endif()

    # 链接静态库和链接脚本 (由于启动文件已被移动至 APP_SOURCES，此处可以按需链接，丢弃 --whole-archive)
    # 若需使用驱动，将组件名称（如 component_dummy_sensor）追加在下方
    target_link_libraries(${PROJECT_NAME}.elf PRIVATE component_oled component_dummy_sensor component_tb6612 component_gray_adc component_control component_easy_menu third_party_lib driverlib)
    target_link_options(${PROJECT_NAME}.elf PRIVATE -T ${LINKER_SCRIPT})

    # 生成 bin/hex 辅助构建目标，并输出 size 统计
    add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
        COMMENT "Generating BIN, HEX and printing executable size..."
    )
else()
    message(FATAL_ERROR "This project only supports MCU firmware builds. Configure with -DCMAKE_TOOLCHAIN_FILE=\"gcc-arm-none-eabi.cmake\".")
endif()
