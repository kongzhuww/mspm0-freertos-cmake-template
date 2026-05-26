# ==========================================
# 比赛驱动组件模板 (Dummy Sensor)
# 复制本文件夹即可快速创建新驱动模块
# ==========================================

# 1. 搜集本组件下的所有源码
file(GLOB COMPONENT_SRCS "src/*.c")

# 2. 将组件打包为一个静态库
# 命名规范: component_<文件夹名>
add_library(component_dummy_sensor STATIC ${COMPONENT_SRCS})

# 3. 暴露本组件的头文件目录
# 使用 PUBLIC，这意味着任何链接了 component_dummy_sensor 的目标，都可以直接 #include "dummy_sensor.h"
target_include_directories(component_dummy_sensor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4. (可选) 声明依赖的其他底层库
# 如果组件内部调用了 DriverLib 硬件库或者 FreeRTOS，可以通过 PRIVATE 链接，从而不污染上层
if(CMAKE_CROSSCOMPILING)
    target_link_libraries(component_dummy_sensor PRIVATE third_party_lib)
endif()
