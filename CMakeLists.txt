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
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,bugprone-*")
endif()

if(CMAKE_CROSSCOMPILING)
    # --------- 交叉编译到单片机 ---------
    
    # 设置目标 CPU 和交叉编译标志
    set(CPU_FLAGS "-mcpu=cortex-m0plus -mthumb")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS} -ffunction-sections -fdata-sections -Wall -O2")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_FLAGS} -x assembler-with-cpp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections")

    # 全局宏定义
    add_definitions(-D__MSPM0G3507__)

    # 包含路径
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/third_party/CMSIS/Core/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/portable/GCC/ARM_CM0
    )

    # 源文件
    set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/empty.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ti_msp_dl_config.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/board_led.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0_Debug.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tim_delay.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app_control.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/devices/startup_mspm0g350x_gcc.c
        
        # FreeRTOS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/croutine.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/event_groups.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/stream_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/tasks.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/src/timers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/portable/GCC/ARM_CM0/port.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/portable/MemMang/heap_4.c
    )

    # 静态库路径和链接
    set(DRIVERLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk_source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a")

    # 链接脚本
    set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/devices/mspm0g3507.lds")

    add_executable(${PROJECT_NAME}.elf ${SOURCES})

    # 链接静态库和链接脚本
    target_link_libraries(${PROJECT_NAME}.elf PRIVATE ${DRIVERLIB_PATH})
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
    message(STATUS "Host test build mode")
    
    # 启用测试
    enable_testing()
    
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unity
    )
    
    set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app_control.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unity/unity.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_app_control.c
    )
    
    add_executable(run_tests ${TEST_SOURCES})
    add_test(NAME run_tests COMMAND run_tests)
    
endif()
