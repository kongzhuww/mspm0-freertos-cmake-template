cmake_minimum_required(VERSION 3.20)
project(MSPM0G3507_FreeRTOS C ASM CXX)

# 支持 C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 启用 GCC JSON 格式编译错误输出
add_compile_options(-fdiagnostics-format=json)

# 启用 clang-tidy (如果可用)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    # 不再全局设置，而是保存配置，后续按 Target 应用
    set(MY_CLANG_TIDY_CMD "${CLANG_TIDY_EXE};-checks=-*,readability-*,bugprone-*")
endif()

if(CMAKE_CROSSCOMPILING)
    # --------- 交叉编译到单片机 ---------
    
    # 设置目标 CPU 和交叉编译标志
    set(CPU_FLAGS "-mcpu=cortex-m0plus -mthumb")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS} -g -ffunction-sections -fdata-sections -Wall")
    set(CMAKE_C_FLAGS_DEBUG "-O0")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_FLAGS} -x assembler-with-cpp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections")

    # (已移除全局 add_definitions 和 include_directories，改为 Target 精准配置)


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

    # 静态库路径和链接
    set(DRIVERLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a")

    # 链接脚本
    set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/devices/mspm0g3507.lds")

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

    # 仅将业务代码单独开启 clang-tidy 静态检查
    if(CLANG_TIDY_EXE)
        set_target_properties(${PROJECT_NAME}.elf PROPERTIES C_CLANG_TIDY "${MY_CLANG_TIDY_CMD}")
    endif()

    # 链接静态库和链接脚本 (由于启动文件已被移动至 APP_SOURCES，此处可以按需链接，丢弃 --whole-archive)
    target_link_libraries(${PROJECT_NAME}.elf PRIVATE third_party_lib ${DRIVERLIB_PATH})
    target_link_options(${PROJECT_NAME}.elf PRIVATE -T ${LINKER_SCRIPT})

    # 生成 bin/hex 辅助构建目标，并输出 size 统计
    add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
        COMMENT "Generating BIN, HEX and printing executable size..."
    )
else()
    # --------- 本机编译及测试 ---------
    message(STATUS "PC test build mode")
    
    # 启用测试
    enable_testing()
    
    # (已移除全局 include_directories，改为 Target 精准配置)
    
    # 将 Unity 框架剥离为静态库，免受 clang-tidy 打扰
    add_library(unity_lib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/test/unity/unity.c)
    # 为 Unity 库配置头文件，并设为 PUBLIC 传递给依赖它的测试程序
    target_include_directories(unity_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unity
    )
    
    set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app_control.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_app_control.c
    )
    
    add_executable(run_tests ${TEST_SOURCES})
    # 为测试程序配置业务头文件
    target_include_directories(run_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(run_tests PRIVATE unity_lib)

    # 仅将单元测试及业务代码单独开启 clang-tidy 静态检查
    if(CLANG_TIDY_EXE)
        set_target_properties(run_tests PROPERTIES C_CLANG_TIDY "${MY_CLANG_TIDY_CMD}")
    endif()

    add_test(NAME run_tests COMMAND run_tests)
    
endif()
