# ==========================================
# 比赛驱动组件 (OLED 7-Pin SPI 模拟驱动)
# ==========================================

# 1. 搜集本组件下的所有源码
file(GLOB COMPONENT_SRCS "src/*.c")

# 2. 将组件打包为一个静态库
add_library(component_oled STATIC ${COMPONENT_SRCS})

# 3. 暴露本组件的头文件目录
target_include_directories(component_oled PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4. 声明依赖的底层库
if(CMAKE_CROSSCOMPILING)
    target_link_libraries(component_oled PRIVATE third_party_lib)
endif()
